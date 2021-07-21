/* a simple calculator written in C99 */
#include "calculator.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

/* TODO: add int support */

#define CALCULATOR_DEBUG

#define CALCULATOR_ERROR(message) \
    { \
        fprintf(stderr, message "\n"); \
        exit(1); \
    }

/* token names */
enum TokenType {
    TokenTypeNumber = 1,
    TokenTypePlus,
    TokenTypeMin,
    TokenTypeMul,
    TokenTypeDiv,
    TokenTypeOpenParen,
    TokenTypeCloseParen,
    TokenTypeCaret,
    TokenTypePercent,
    TokenTypeEof
};

/* a more complex token, that also has an optional value */
struct Token {
    enum TokenType type;
    double value;
};

/* a helper struct that manages the tokenization stage */
struct Tokenizer {
    char *stream;
    size_t idx;
    struct Token *token_array;
    size_t allocated;
    size_t write_idx;
};

/* node tree types */
enum NodeType {
    NodeTypeNumber = 1,
    NodeTypePlus,
    NodeTypeMin,
    NodeTypeMul,
    NodeTypeDiv,
    NodeTypeMod,
    NodeTypeParen,
    NodeTypePow,
    NodeTypeNeg
};

/* a complex and recursive node that stores a number or children nodes */
struct Node {
    enum NodeType type;
    union {
        double number;
        struct {
            struct Node *lhs, *rhs;
        } children;
        struct Node *in_paren;
        struct Node *to_negative;
    } value;
};

/* a struct that manages the parsing stage */
struct Parser {
    struct Token *tokens;
    struct Node *write;
};

static void tokenizer_append_token(struct Tokenizer *tokenizer, struct Token token) {
    if (tokenizer->write_idx >= tokenizer->allocated) {
        tokenizer->allocated += 8;
        tokenizer->token_array = realloc(tokenizer->token_array, tokenizer->allocated * sizeof(struct Token));
    }
    tokenizer->token_array[tokenizer->write_idx++] = token;
}

static struct Token *tokenize(char *stream) {
    struct Tokenizer tokenizer;

    if (stream == NULL) return NULL;
    tokenizer.stream = stream;
    tokenizer.token_array = NULL;
    tokenizer.idx = 0;
    tokenizer.allocated = tokenizer.write_idx = 0;

    while (tokenizer.stream[tokenizer.idx] != '\0') {
        switch (tokenizer.stream[tokenizer.idx]) {
        case '+':
            tokenizer_append_token(&tokenizer, (struct Token) { TokenTypePlus });
            ++tokenizer.idx;
            break;
        case '-':
            tokenizer_append_token(&tokenizer, (struct Token) { TokenTypeMin });
            ++tokenizer.idx;
            break;
        case '*':
            tokenizer_append_token(&tokenizer, (struct Token) { TokenTypeMul });
            ++tokenizer.idx;
            break;
        case '/':
            tokenizer_append_token(&tokenizer, (struct Token) { TokenTypeDiv });
            ++tokenizer.idx;
            break;
        case '%':
            tokenizer_append_token(&tokenizer, (struct Token) { TokenTypePercent });
            ++tokenizer.idx;
            break;
        case '(':
            tokenizer_append_token(&tokenizer, (struct Token) { TokenTypeOpenParen });
            ++tokenizer.idx;
            break;
        case ')':
            tokenizer_append_token(&tokenizer, (struct Token) { TokenTypeCloseParen });
            ++tokenizer.idx;
            break;
        case '^':
            tokenizer_append_token(&tokenizer, (struct Token) { TokenTypeCaret });
            ++tokenizer.idx;
            break;
        case ' ':
            ++tokenizer.idx;
            break;
        default: {
            char *end;
            double n = strtod(&tokenizer.stream[tokenizer.idx], &end);

            if (&tokenizer.stream[tokenizer.idx] == end) {
                free(tokenizer.token_array);
                CALCULATOR_ERROR("UNKNOWN IDENTIFIER");
            }

            tokenizer_append_token(&tokenizer, (struct Token) { TokenTypeNumber, n});
            tokenizer.idx += end - &tokenizer.stream[tokenizer.idx];
            break;
        }
        }
    }

    tokenizer_append_token(&tokenizer, (struct Token) { TokenTypeEof });
    return tokenizer.token_array;
}

static void free_node(struct Node *node) {
    double res;
    switch (node->type) {
    case NodeTypePlus:
    case NodeTypeMin:
    case NodeTypeMul:
    case NodeTypeDiv:
        free_node(node->value.children.lhs);
        free_node(node->value.children.rhs);
        break;
    case NodeTypeParen:
        free_node(node->value.in_paren);
    }
    // free the node itself
    free(node);
} 

static void parse_node(struct Parser *parser);

static struct Node *parse_value(struct Parser *parser) {
    struct Node *new_node, *tmp;
    bool negative;
    if (parser->tokens[0].type == TokenTypeMin) {
        ++parser->tokens;
        negative = true;
    } else {
        negative = false;
    }

    switch (parser->tokens[0].type) {
    case TokenTypeNumber:
        new_node = malloc(sizeof(struct Node));
        *new_node = (struct Node) {
            .type = NodeTypeNumber,
            .value.number = parser->tokens[0].value
        };
        ++parser->tokens;
        break;
    case TokenTypeOpenParen: {
        struct Node *top_node = parser->write;
        parser->write = NULL;
        // advance past '('
        ++parser->tokens;
        // try and parse the first value, on fail throw an error
        if ((parser->write = parse_value(parser)) == NULL) {
            CALCULATOR_ERROR("CAN'T START AN EXPRESSION WITH A NON-NUMBER");
        }
        while (parser->tokens[0].type != TokenTypeCloseParen) {
            if (parser->tokens[0].type == TokenTypeEof) {
                free_node(top_node);
                CALCULATOR_ERROR("UNMATCHED '('");
            }
            parse_node(parser);
        }
        // advance past ')'
        ++parser->tokens;
        new_node = malloc(sizeof(struct Node));
        *new_node = (struct Node) {
            .type = NodeTypeParen,
            .value.in_paren = parser->write
        };
        parser->write = top_node;
        break;
    }
    default:
        if (negative) --parser->tokens;
        return NULL;
    }
    if (negative) {
        tmp = new_node;
        new_node = malloc(sizeof(struct Node));
        *new_node = (struct Node) {
            .type = NodeTypeNeg,
            .value.to_negative = tmp
        };
    }
    return new_node;
}

static void parse_node(struct Parser *parser) {
    struct Node *new_node;
    enum NodeType new_node_type;
    struct Node **write_node = &parser->write;

    switch (parser->tokens[0].type) {
    case TokenTypeNumber:
    case TokenTypeOpenParen:
        free_node(parser->write);
        CALCULATOR_ERROR("NO OPERATOR BETWEEN EXPRESSIONS");
        return;
    case TokenTypeCloseParen:
        free_node(parser->write);
        CALCULATOR_ERROR("UNMATCHED ')'");
        return;
    case TokenTypePlus:
        if (!parser->write) {
            CALCULATOR_ERROR("CAN'T START EXPRESSION WITH A NON-NUMBER");
        }
        new_node_type = NodeTypePlus;
        break;
    case TokenTypeMin:
        if (!parser->write) {
            CALCULATOR_ERROR("CAN'T START EXPRESSION WITH A NON-NUMBER");
        }
        new_node_type = NodeTypeMin;
        break;
    case TokenTypeMul:
        if (!parser->write) {
            CALCULATOR_ERROR("CAN'T START EXPRESSION WITH A NON-NUMBER");
        }
        new_node_type = NodeTypeMul;
        switch (parser->write->type) {
        case NodeTypePlus:
        case NodeTypeMin:
            write_node = &parser->write->value.children.rhs;
            break;
        }
        break;
    case TokenTypeDiv:
        if (!parser->write) {
            CALCULATOR_ERROR("CAN'T START EXPRESSION WITH A NON-NUMBER");
        }
        new_node_type = NodeTypeDiv;
        switch (parser->write->type) {
        case NodeTypePlus:
        case NodeTypeMin:
            write_node = &parser->write->value.children.rhs;
            break;
        }
        break;
    case TokenTypePercent:
        if (!parser->write) {
            CALCULATOR_ERROR("CAN'T START EXPRESSION WITH A NON-NUMBER");
        }
        new_node_type = NodeTypeMod;
        switch (parser->write->type) {
        case NodeTypePlus:
        case NodeTypeMin:
            write_node = &parser->write->value.children.rhs;
            break;
        }
        break;
    case TokenTypeCaret:
        if (!parser->write) {
            CALCULATOR_ERROR("CAN'T START EXPRESSION WITH A NON-NUMBER");
        }
        new_node_type = NodeTypePow;
        switch (parser->write->type) {
        case NodeTypePlus:
        case NodeTypeMin:
            // save current working node
            new_node = parser->write;
            parser->write = parser->write->value.children.rhs;
            parse_node(parser);
            new_node->value.children.rhs = parser->write;
            // load current working node
            parser->write = new_node;
            return;
        case NodeTypeMul:
        case NodeTypeDiv:
        case NodeTypeMod:
            write_node = &parser->write->value.children.rhs;
            break;
        }
        break;
    }
    ++parser->tokens;
    new_node = malloc(sizeof(struct Node));
    new_node->type = new_node_type;
    new_node->value.children.lhs = *write_node;
    if ((new_node->value.children.rhs = parse_value(parser)) == NULL) {
        free_node(parser->write);
        free(new_node);
        CALCULATOR_ERROR("NO OPERATOR BETWEEN EXPRESSIONS");
    }
    *write_node = new_node;
}

static struct Node *parse(struct Token *tokens) {
    struct Parser parser;
    parser.tokens = tokens;
    parser.write = NULL;

    // if you start with a non number or an eof, it is an error
    if (tokens == NULL) return NULL;
    if ((parser.write = parse_value(&parser)) == NULL) {
        free(tokens);
        return NULL;
    }

    while (parser.tokens[0].type != TokenTypeEof) {
        parse_node(&parser);
    }

    free(tokens);
    return parser.write;
}

static double eval(struct Node *node) {
    double res;
    switch (node->type) {
    case NodeTypeNumber:
        res = node->value.number;
        break;
    case NodeTypePlus:
        res = eval(node->value.children.lhs) + eval(node->value.children.rhs);
        break;
    case NodeTypeMin:
        res = eval(node->value.children.lhs) - eval(node->value.children.rhs);
        break;
    case NodeTypeMul:
        res = eval(node->value.children.lhs) * eval(node->value.children.rhs);
        break;
    case NodeTypeDiv:
        res = eval(node->value.children.rhs);
        if (res == 0.0f) {
            free_node(node->value.children.lhs);
            free(node);
            CALCULATOR_ERROR("DIVISION BY ZERO");
        }
        res = eval(node->value.children.lhs) / res;
        break;
    case NodeTypeMod:
        res = eval(node->value.children.rhs);
        if (res == 0.0f) {
            free_node(node->value.children.lhs);
            free(node);
            CALCULATOR_ERROR("DIVISION BY ZERO");
        }
        res = fmod(eval(node->value.children.lhs), res);
        break;
    case NodeTypePow:
        res = powf(eval(node->value.children.lhs), eval(node->value.children.rhs));
        break;
    case NodeTypeParen:
        res = eval(node->value.in_paren);
        break;
    case NodeTypeNeg:
        res = -eval(node->value.to_negative);
        break;
    }
    // free the node
    free(node);
    return res;
}

static void print_ast(struct Node *node) {
    switch (node->type) {
    case NodeTypeNumber:
        printf("%f", node->value.number);
        break;
    case NodeTypePlus:
        print_ast(node->value.children.lhs);
        printf("+");
        print_ast(node->value.children.rhs);
        break;
    case NodeTypeMin:
        print_ast(node->value.children.lhs);
        printf("-");
        print_ast(node->value.children.rhs);
        break;
    case NodeTypeMul:
        print_ast(node->value.children.lhs);
        printf("*");
        print_ast(node->value.children.rhs);
        break;
    case NodeTypeDiv:
        print_ast(node->value.children.lhs);
        printf("/");
        print_ast(node->value.children.rhs);
        break;
    case NodeTypeMod:
        print_ast(node->value.children.lhs);
        printf("%%");
        print_ast(node->value.children.rhs);
        break;
    case NodeTypePow:
        print_ast(node->value.children.lhs);
        printf("^");
        print_ast(node->value.children.rhs);
        break;
    case NodeTypeParen:
        printf("(");
        print_ast(node->value.in_paren);
        printf(")");
        break;
    case NodeTypeNeg:
        printf("-");
        print_ast(node->value.to_negative);
        break;
    }
}

double calculate(char *stream) {
    struct Token *tokens;
    struct Node *ast;
    if ((tokens = tokenize(stream)) == NULL)
        return 0.0f;
    if ((ast = parse(tokens)) == NULL)
        return 0.0f;
#ifdef CALCULATOR_DEBUG
    print_ast(ast);
    printf("\n");
#endif
    return eval(ast);
}

