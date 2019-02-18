/* db_generate.c - code generation functions
 *
 * Copyright (c) 2009 by David Michael Betz.  All rights reserved.
 *
 */

#include "db_compiler.h"

/* local function prototypes */
static void code_expr(ParseContext *c, ParseTreeNode *expr, PVAL *pv);
static void code_shortcircuit(ParseContext *c, int op, ParseTreeNode *expr, PVAL *pv);
static void code_arrayref(ParseContext *c, ParseTreeNode *expr, PVAL *pv);
static void code_call(ParseContext *c, ParseTreeNode *expr, PVAL *pv);
static void code_index(ParseContext *c, PValOp fcn, PVAL *pv);
static VMVALUE rd_cword(ParseContext *c, VMUVALUE off);
static void wr_cword(ParseContext *c, VMUVALUE off, VMVALUE w);

/* code_lvalue - generate code for an l-value expression */
void code_lvalue(ParseContext *c, ParseTreeNode *expr, PVAL *pv)
{
    code_expr(c, expr, pv);
    chklvalue(c, pv);
}

/* code_rvalue - generate code for an r-value expression */
void code_rvalue(ParseContext *c, ParseTreeNode *expr)
{
    PVAL pv;
    code_expr(c, expr, &pv);
    rvalue(c, &pv);
}

/* code_expr - generate code for an expression parse tree */
static void code_expr(ParseContext *c, ParseTreeNode *expr, PVAL *pv)
{
    VMVALUE ival;
    switch (expr->nodeType) {
    case NodeTypeSymbolRef:
        pv->fcn = expr->u.symbolRef.fcn;
        if (pv->fcn == code_global)
            pv->u.sym = expr->u.symbolRef.symbol;
        else
            pv->u.val = expr->u.symbolRef.offset;
        break;
    case NodeTypeStringLit:
        putcbyte(c, OP_LIT);
        putcword(c, AddStringRef(expr->u.stringLit.string, codeaddr(c)));
        pv->fcn = NULL;
        break;
    case NodeTypeIntegerLit:
        ival = expr->u.integerLit.value;
        if (ival >= -128 && ival <= 127) {
            putcbyte(c, OP_SLIT);
            putcbyte(c, ival);
        }
        else {
            putcbyte(c, OP_LIT);
            putcword(c, ival);
        }
        pv->fcn = NULL;
        break;
    case NodeTypeFunctionLit:
        putcbyte(c, OP_LIT);
        putcword(c, expr->u.functionLit.offset);
        pv->fcn = NULL;
        break;
    case NodeTypeUnaryOp:
        code_rvalue(c, expr->u.unaryOp.expr);
        putcbyte(c, expr->u.unaryOp.op);
        pv->fcn = NULL;
        break;
    case NodeTypeBinaryOp:
        code_rvalue(c, expr->u.binaryOp.left);
        code_rvalue(c, expr->u.binaryOp.right);
        putcbyte(c, expr->u.binaryOp.op);
        pv->fcn = NULL;
        break;
    case NodeTypeArrayRef:
        code_arrayref(c, expr, pv);
        break;
    case NodeTypeFunctionCall:
        code_call(c, expr, pv);
        break;
    case NodeTypeDisjunction:
        code_shortcircuit(c, OP_BRTSC, expr, pv);
        break;
    case NodeTypeConjunction:
        code_shortcircuit(c, OP_BRFSC, expr, pv);
        break;
    }
}

/* code_shortcircuit - generate code for a conjunction or disjunction of boolean expressions */
static void code_shortcircuit(ParseContext *c, int op, ParseTreeNode *expr, PVAL *pv)
{
    ExprListEntry *entry = expr->u.exprList.exprs;
    int end = 0;

    code_rvalue(c, entry->expr);
    entry = entry->next;

    do {
        putcbyte(c, op);
        end = putcword(c, end);
        code_rvalue(c, entry->expr);
    } while ((entry = entry->next) != NULL);

    fixupbranch(c, end, codeaddr(c));

    pv->fcn = NULL;
}

/* code_arrayref - code an array reference */
static void code_arrayref(ParseContext *c, ParseTreeNode *expr, PVAL *pv)
{
    PVAL pv2;

    /* code the array address */
    code_lvalue(c, expr->u.arrayRef.array, &pv2);
    (*pv2.fcn)(c, PV_ADDR, &pv2);

    /* code the index */
    code_rvalue(c, expr->u.arrayRef.index);

    /* code the index operation */
    putcbyte(c, OP_INDEX);

    /* setup the element type */
    pv->fcn = code_index;
}

/* code_call - code a function call */
static void code_call(ParseContext *c, ParseTreeNode *expr, PVAL *pv)
{
    ExprListEntry *arg;

    /* code each argument expression */
    for (arg = expr->u.functionCall.args; arg != NULL; arg = arg->next)
        code_rvalue(c, arg->expr);

    /* call the function */
    code_rvalue(c, expr->u.functionCall.fcn);
    putcbyte(c, OP_CALL);
    putcbyte(c, expr->u.functionCall.argc);

    /* we've got an rvalue now */
    pv->fcn = NULL;
}

/* rvalue - get the rvalue of a partial expression */
void rvalue(ParseContext *c, PVAL *pv)
{
    if (pv->fcn) {
        (*pv->fcn)(c, PV_LOAD, pv);
        pv->fcn = NULL;
    }
}

/* chklvalue - make sure we've got an lvalue */
void chklvalue(ParseContext *c, PVAL *pv)
{
    if (pv->fcn == NULL)
        ParseError(c,"Expecting an lvalue");
}

/* code_global - compile a global variable reference */
void code_global(ParseContext *c, PValOp fcn, PVAL *pv)
{
    putcbyte(c, OP_LIT);
    putcword(c, AddSymbolRef(pv->u.sym, codeaddr(c)));
    switch (fcn) {
    case PV_ADDR:
        // just need the address
        break;
    case PV_LOAD:
        putcbyte(c, OP_LOAD);
        break;
    case PV_STORE:
        putcbyte(c, OP_STORE);
        break;
    }
}

/* code_local - compile an local reference */
void code_local(ParseContext *c, PValOp fcn, PVAL *pv)
{
    switch (fcn) {
    case PV_ADDR:
        // what to do here?
        break;
    case PV_LOAD:
        putcbyte(c, OP_LREF);
        putcbyte(c, pv->u.val);
        break;
    case PV_STORE:
        putcbyte(c, OP_LSET);
        putcbyte(c, pv->u.val);
        break;
    }
}

/* code_index - compile a vector reference */
static void code_index(ParseContext *c, PValOp fcn, PVAL *pv)
{
    switch (fcn) {
    case PV_ADDR:
        // what to do here?
        break;
    case PV_LOAD:
        putcbyte(c, OP_LOAD);
        break;
    case PV_STORE:
        putcbyte(c, OP_STORE);
        break;
    }
}

/* codeaddr - get the current code address (actually, offset) */
int codeaddr(ParseContext *c)
{
    return (int)(c->cptr - c->codeBuf);
}

/* putcbyte - put a code byte into the code buffer */
int putcbyte(ParseContext *c, int b)
{
    int addr = codeaddr(c);
    if (c->cptr >= c->ctop)
        Abort(c->sys, "Bytecode buffer overflow");
    *c->cptr++ = b;
    return addr;
}

/* putcword - put a code word into the code buffer */
int putcword(ParseContext *c, VMVALUE w)
{
    int addr = codeaddr(c);
    if (c->cptr + sizeof(VMVALUE) > c->ctop)
        Abort(c->sys, "Bytecode buffer overflow");
    wr_cword(c, c->cptr - c->codeBuf, w);
    c->cptr += sizeof(VMVALUE);
    return addr;
}

/* rd_cword - get a code word from the code buffer */
VMVALUE rd_cword(ParseContext *c, VMUVALUE off)
{
    int cnt = sizeof(VMVALUE);
    VMVALUE w = 0;
    while (--cnt >= 0)
        w = (w << 8) | c->codeBuf[off++];
    return w;
}

/* wr_cword - put a code word into the code buffer */
void wr_cword(ParseContext *c, VMUVALUE off, VMVALUE w)
{
    uint8_t *p = &c->codeBuf[off] + sizeof(VMVALUE);
    int cnt = sizeof(VMVALUE);
    while (--cnt >= 0) {
        *--p = w;
        w >>= 8;
    }
}

/* fixup - fixup a reference chain */
void fixup(ParseContext *c, VMUVALUE chn, VMUVALUE val)
{
    while (chn != 0) {
        int nxt = rd_cword(c, chn);
        wr_cword(c, chn, val);
        chn = nxt;
    }
}

/* fixupbranch - fixup a reference chain */
void fixupbranch(ParseContext *c, VMUVALUE chn, VMUVALUE val)
{
    while (chn != 0) {
        int nxt = rd_cword(c, chn);
        int off = val - (chn + sizeof(VMUVALUE)); /* this assumes all 1+sizeof(VMUVALUE) byte branch instructions */
        wr_cword(c, chn, off);
        chn = nxt;
    }
}
