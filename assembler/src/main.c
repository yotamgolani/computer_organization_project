/* asm.c — SIMP assembler (2-pass) producing memin.txt
   Usage: asm.exe program.asm memin.txt
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define MEM_SIZE          4096
#define WORD_MASK         0xFFFFF  /* 20-bit */
#define MAX_LINE_LENGTH   500
#define MAX_LABEL_LENGTH  50
#define MAX_TOKENS        32

typedef struct {
    char name[MAX_LABEL_LENGTH];
    int  addr; /* memory word address (PC value / memory word index) */
} Symbol;

typedef struct {
    Symbol *syms;
    size_t  count;
    size_t  cap;
} Symtab;

static void die_at(int line_no, const char *msg) {
    if (line_no > 0) fprintf(stderr, "Error (line %d): %s\n", line_no, msg);
    else fprintf(stderr, "Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

static void *xrealloc(void *p, size_t n) {
    void *q = realloc(p, n);
    if (!q) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
    return q;
}

/* -------- string helpers -------- */

static char *ltrim(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    return s;
}

static void rtrim_inplace(char *s) {
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) {
        s[n - 1] = '\0';
        n--;
    }
}

static void strtolower_inplace(char *s) {
    while (*s) {
        *s = (char)tolower((unsigned char)*s);
        s++;
    }
}

/* -------- parsing helpers -------- */

static int is_valid_label_name(const char *name) {
    size_t i;
    if (!name || !isalpha((unsigned char)name[0])) return 0;
    for (i = 1; name[i]; i++) {
        if (!isalnum((unsigned char)name[i])) return 0;
    }
    return 1;
}

static int is_label_def_token(const char *tok, char *out_label /* no ':' */) {
    size_t len, copy;
    char tmp[MAX_LABEL_LENGTH];

    if (!tok) return 0;
    len = strlen(tok);
    if (len < 2) return 0;
    if (tok[len - 1] != ':') return 0;

    copy = (len - 1 < (size_t)(MAX_LABEL_LENGTH - 1)) ? (len - 1) : (size_t)(MAX_LABEL_LENGTH - 1);
    memcpy(tmp, tok, copy);
    tmp[copy] = '\0';

    if (!is_valid_label_name(tmp)) return 0;
    if (out_label) strcpy(out_label, tmp);
    return 1;
}

static int parse_num20(const char *tok, int *out_val) {
    /* keep low 20 bits */
    char *end;
    long v;

    if (!tok || !out_val) return 0;
    errno = 0;
    end = NULL;
    v = strtol(tok, &end, 0);
    if (errno != 0) return 0;
    if (end == tok || *end != '\0') return 0;

    *out_val = ((int)v) & WORD_MASK;
    return 1;
}

/* Tokenize a line (comment already cut):
   - commas become spaces
   - split on whitespace
*/
static int tokenize(char *line, char *toks[], int max_toks) {
    int n;
    char *p;
    char *tok;

    for (p = line; *p; p++) {
        if (*p == ',') *p = ' ';
    }

    n = 0;
    tok = strtok(line, " \t\r\n");
    while (tok && n < max_toks) {
        toks[n++] = tok;
        tok = strtok(NULL, " \t\r\n");
    }
    return n;
}

/* Validate comment placement rule and cut it.
   - Determine line type from tokens BEFORE '#'
   - Enforce exact required token count for that type.
   - Allow '#' after last operand
    cut at the first '#'.
*/
static void validate_and_cut_comment(char *line, int line_no) {
    char *hash;
    char prefix[MAX_LINE_LENGTH];
    char tmp[MAX_LINE_LENGTH];
    char *toks[MAX_TOKENS];
    int nt;
    int tok_i;
    char lbl[MAX_LABEL_LENGTH];

    if (strchr(line, ';') != NULL) die_at(line_no, "only '#' comments are allowed (found ';')");

    hash = strchr(line, '#');
    if (hash) {
        size_t n = (size_t)(hash - line);
        if (n >= sizeof(prefix)) n = sizeof(prefix) - 1;
        memcpy(prefix, line, n);
        prefix[n] = '\0';
        *hash = '\0'; /* cut for downstream */
    } else {
        strncpy(prefix, line, sizeof(prefix) - 1);
        prefix[sizeof(prefix) - 1] = '\0';
    }

    rtrim_inplace(prefix);
    /* If the line is empty before comment, it's fine */
    if (ltrim(prefix)[0] == '\0') return;

    strncpy(tmp, prefix, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
    nt = tokenize(tmp, toks, MAX_TOKENS);
    if (nt == 0) return;

    tok_i = 0;
    if (is_label_def_token(toks[0], lbl)) {
        tok_i = 1;
        if (tok_i >= nt) {
            return;
        }
        if (strcmp(toks[tok_i], ".word") == 0) {
            die_at(line_no, "label before .word is not allowed (use: label: on its own line, or .word without label)");
        }
        if (nt != 6) die_at(line_no, "comment must be after the 5th operand (instruction has: opcode rd rs rt imm)");
        return;
    }

    /* No label */
    if (strcmp(toks[0], ".word") == 0) {
        if (nt != 3) die_at(line_no, "comment must be after: .word address data");
        return;
    }

    /* Instruction without label must be exactly 5 tokens */
    if (nt != 5) die_at(line_no, "comment must be after the 5th operand (instruction has: opcode rd rs rt imm)");
}

/* -------- symtab -------- */

static void symtab_init(Symtab *st) {
    st->syms = NULL;
    st->count = 0;
    st->cap = 0;
}

static void symtab_free(Symtab *st) {
    free(st->syms);
    st->syms = NULL;
    st->count = 0;
    st->cap = 0;
}

static int symtab_find(const Symtab *st, const char *name) {
    size_t i;
    for (i = 0; i < st->count; i++) {
        if (strcmp(st->syms[i].name, name) == 0) return (int)i;
    }
    return -1;
}

static void symtab_add(Symtab *st, const char *name, int addr, int line_no) {
    size_t idx;

    if (symtab_find(st, name) >= 0) die_at(line_no, "duplicate label");

    if (st->count == st->cap) {
        st->cap = (st->cap == 0) ? 64 : (st->cap * 2);
        st->syms = (Symbol *)xrealloc(st->syms, st->cap * sizeof(Symbol));
    }

    idx = st->count;
    strncpy(st->syms[idx].name, name, (size_t)(MAX_LABEL_LENGTH - 1));
    st->syms[idx].name[MAX_LABEL_LENGTH - 1] = '\0';
    st->syms[idx].addr = addr;
    st->count++;
}

static int symtab_addr(const Symtab *st, const char *name, int *out_addr) {
    int idx = symtab_find(st, name);
    if (idx < 0) return 0;
    *out_addr = st->syms[(size_t)idx].addr;
    return 1;
}

/* -------- ISA tables -------- */

typedef struct { const char *name; int num; } NameNum;

static const NameNum reg_table[] = {
    {"$zero", 0}, {"$imm",  1}, {"$v0", 2}, {"$a0", 3}, {"$a1", 4}, {"$a2", 5}, {"$a3", 6},
    {"$t0", 7}, {"$t1", 8}, {"$t2", 9},
    {"$s0", 10}, {"$s1", 11}, {"$s2", 12},
    {"$gp", 13}, {"$sp", 14}, {"$ra", 15}
};

static const NameNum op_table[] = {
    {"add",  0}, {"sub",  1}, {"mul",  2}, {"and",  3}, {"or",   4}, {"xor",  5},
    {"sll",  6}, {"sra",  7}, {"srl",  8},
    {"beq",  9}, {"bne", 10}, {"blt", 11}, {"bgt", 12}, {"ble", 13}, {"bge", 14},
    {"jal", 15},
    {"lw",  16}, {"sw",  17},
    {"reti",18},
    {"in",  19}, {"out", 20},
    {"halt",21}
};

static int lookup_name_num(const NameNum *tab, size_t n, const char *name, int *out) {
    size_t i;
    for (i = 0; i < n; i++) {
        if (strcmp(tab[i].name, name) == 0) {
            *out = tab[i].num;
            return 1;
        }
    }
    return 0;
}

static int parse_reg(const char *tok, int *out_reg) {
    char tmp[64];
    strncpy(tmp, tok, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
    strtolower_inplace(tmp);
    return lookup_name_num(reg_table, sizeof(reg_table)/sizeof(reg_table[0]), tmp, out_reg);
}

static int parse_opcode(const char *tok, int *out_op) {
    char tmp[64];
    strncpy(tmp, tok, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
    strtolower_inplace(tmp);
    return lookup_name_num(op_table, sizeof(op_table)/sizeof(op_table[0]), tmp, out_op);
}

static int is_I_format(int rd, int rs, int rt) {
    return (rd == 1 || rs == 1 || rt == 1);
}

/* opcode(8) | rd(4) | rs(4) | rt(4) */
static int encode_inst(int opcode, int rd, int rs, int rt) {
    int w = ((opcode & 0xFF) << 12) | ((rd & 0xF) << 8) | ((rs & 0xF) << 4) | (rt & 0xF);
    return w & WORD_MASK;
}

/* -------- Pass 1 -------- */

static void pass1_build_labels(FILE *in, Symtab *st) {
    char line[MAX_LINE_LENGTH];
    int pc = 0;
    int line_no = 0;

    while (fgets(line, sizeof(line), in)) {
        char *p;
        char tmp[MAX_LINE_LENGTH];
        char *toks[MAX_TOKENS];
        int nt;
        int tok_i;
        char lbl[MAX_LABEL_LENGTH];

        line_no++;
        line[strcspn(line, "\n")] = '\0';

        validate_and_cut_comment(line, line_no);
        rtrim_inplace(line);
        p = ltrim(line);
        if (*p == '\0') continue;

        strncpy(tmp, p, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = '\0';
        nt = tokenize(tmp, toks, MAX_TOKENS);
        if (nt == 0) continue;

        tok_i = 0;
        if (is_label_def_token(toks[0], lbl)) {
            tok_i = 1;
            if (tok_i >= nt) {
                symtab_add(st, lbl, pc, line_no);
                continue;
            }
            if (strcmp(toks[tok_i], ".word") == 0) {
                die_at(line_no, "label before .word is not allowed (use: label: on its own line, or .word without label)");
            }
            symtab_add(st, lbl, pc, line_no);
        }

        if (strcmp(toks[tok_i], ".word") == 0) {
            /* .word doesn't advance PC */
            continue;
        }

        /* instruction: opcode rd rs rt imm */
        {
            int op, rd, rs, rt;
            if (!parse_opcode(toks[tok_i], &op)) die_at(line_no, "unknown opcode");
            if (!parse_reg(toks[tok_i + 1], &rd)) die_at(line_no, "unknown rd register");
            if (!parse_reg(toks[tok_i + 2], &rs)) die_at(line_no, "unknown rs register");
            if (!parse_reg(toks[tok_i + 3], &rt)) die_at(line_no, "unknown rt register");

            pc += is_I_format(rd, rs, rt) ? 2 : 1;
            if (pc > MEM_SIZE) die_at(line_no, "program too large for memory image");
        }
    }
}

/* -------- Pass 2 -------- */

static void pass2_assemble(FILE *in, const Symtab *st, int mem[MEM_SIZE]) {
    char line[MAX_LINE_LENGTH];
    int pc = 0;
    int line_no = 0;

    while (fgets(line, sizeof(line), in)) {
        char *p;
        char tmp[MAX_LINE_LENGTH];
        char *toks[MAX_TOKENS];
        int nt;
        int tok_i;
        char lbl[MAX_LABEL_LENGTH];

        line_no++;
        line[strcspn(line, "\n")] = '\0';

        validate_and_cut_comment(line, line_no);
        rtrim_inplace(line);
        p = ltrim(line);
        if (*p == '\0') continue;

        strncpy(tmp, p, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = '\0';
        nt = tokenize(tmp, toks, MAX_TOKENS);
        if (nt == 0) continue;

        tok_i = 0;
        if (is_label_def_token(toks[0], lbl)) {
            tok_i = 1;
            if (tok_i >= nt) continue; /* label-only */
            if (strcmp(toks[tok_i], ".word") == 0) {
                die_at(line_no, "label before .word is not allowed (use: label: on its own line, or .word without label)");
            }
        }

        if (strcmp(toks[tok_i], ".word") == 0) {
            int addr20, data20;
            if (!parse_num20(toks[tok_i + 1], &addr20)) die_at(line_no, "bad .word address");
            if (!parse_num20(toks[tok_i + 2], &data20)) die_at(line_no, "bad .word data");
            if (addr20 < 0 || addr20 >= MEM_SIZE) die_at(line_no, ".word address out of range");
            mem[addr20] = data20 & WORD_MASK;
            continue;
        }

        /* instruction */
        {
            int op, rd, rs, rt;
            int inst;

            if (!parse_opcode(toks[tok_i], &op)) die_at(line_no, "unknown opcode");
            if (!parse_reg(toks[tok_i + 1], &rd)) die_at(line_no, "unknown rd register");
            if (!parse_reg(toks[tok_i + 2], &rs)) die_at(line_no, "unknown rs register");
            if (!parse_reg(toks[tok_i + 3], &rt)) die_at(line_no, "unknown rt register");

            inst = encode_inst(op, rd, rs, rt);
            if (pc >= MEM_SIZE) die_at(line_no, "program too large for memory image");
            mem[pc++] = inst;

            if (is_I_format(rd, rs, rt)) {
                int imm20;
                if (parse_num20(toks[tok_i + 4], &imm20)) {
                    /* numeric immediate */
                } else {
                    int addr;
                    if (!is_valid_label_name(toks[tok_i + 4])) die_at(line_no, "bad immediate (not number and not label)");
                    if (!symtab_addr(st, toks[tok_i + 4], &addr)) die_at(line_no, "undefined label in immediate");
                    imm20 = addr & WORD_MASK;
                }

                if (pc >= MEM_SIZE) die_at(line_no, "program too large for memory image");
                mem[pc++] = imm20 & WORD_MASK;
            }
        }
    }
}

/* -------- output -------- */

static void write_memin(const char *path, const int mem[MEM_SIZE]) {
    FILE *out;
    int i;

    out = fopen(path, "w");
    if (!out) {
        perror("fopen memin");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < MEM_SIZE; i++) {
        fprintf(out, "%05x\n", mem[i] & WORD_MASK);
    }
    fclose(out);
}

/* -------- main -------- */

int main(int argc, char **argv) {
    const char *in_path;
    const char *out_path;
    FILE *in;
    Symtab st;
    int mem[MEM_SIZE];
    int i;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s program.asm memin.txt\n", argv[0]);
        return 1;
    }

    in_path = argv[1];
    out_path = argv[2];

    in = fopen(in_path, "r");
    if (!in) {
        perror("fopen input");
        return 1;
    }

    symtab_init(&st);
    pass1_build_labels(in, &st);

    rewind(in);
    for (i = 0; i < MEM_SIZE; i++) mem[i] = 0;

    pass2_assemble(in, &st, mem);

    fclose(in);
    write_memin(out_path, mem);

    symtab_free(&st);
    return 0;
}
