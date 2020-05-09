//
// Created by kroot on 19/7/19.
//

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

Scanner scanner;

typedef struct Node {
    TokenType tokenType;
    struct Node* children[26];
} Node;

static Node* makeNode() {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->tokenType = -1;
    for (int i = 0; i < 26; ++i) {
        newNode->children[i] = NULL;
    }
    return newNode;
}

static Node* root = NULL;

static void createKeywordTrie() {
    static const char* keywords[] = {"and", "class", "def", "else", "false", "for", "if", "nil", "or", "print",
                                     "return", "super", "this", "true", "var", "while"};
    static TokenType keywordTokens[] = {TOKEN_AND, TOKEN_CLASS, TOKEN_DEF, TOKEN_ELSE, TOKEN_FALSE, TOKEN_FOR, TOKEN_IF,
                                        TOKEN_NIL, TOKEN_OR, TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
                                        TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE};
    root = makeNode();
    for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); ++i) {
        Node* p = root;
        int len = strlen(keywords[i]);
        for (int j = 0; j < len; ++j) {
            char cur = keywords[i][j];
            if (p->children[cur - 'a'] == NULL) {
                p->children[cur - 'a'] = makeNode();
            }
            p = p->children[cur - 'a'];
            if (j == len - 1) {
                p->tokenType = keywordTokens[i];
            }
        }
    }
}

void initScanner(const char* source) {
    createKeywordTrie();
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

static bool isDigit(char c) {
    return '0' <= c && c <= '9';
}

static bool isAlpha(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static bool isAtEnd() {
    return *scanner.current == '\0';
}

static char advance() {
    scanner.current++;
    return scanner.current[-1];
}

static bool match(char expected) {
    if (isAtEnd()) return false;
    if (*scanner.current != expected) return false;
    scanner.current++;
    return true;
}

static char peek() {
    return *scanner.current;
}

static char peekNext() {
    if (isAtEnd()) return '\0';
    return scanner.current[1];
}

static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}

static Token errorToken(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;
    return token;
}

static void skipWhitespace() {
    while (1) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t': advance();
                break;
            case '\n': scanner.line++;
                advance();
                break;
            case '/':
                if (peekNext() == '/') {
                    while (peek() != '\n' && !isAtEnd()) advance();
                } else {
                    return;
                }
                break;
            default: return;
        }
    }
}

static Token string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') scanner.line++;
        advance();
    }
    if (isAtEnd()) return errorToken("Unterminated string.");
    advance();
    return makeToken(TOKEN_STRING);
}

static Token number() {
    while (isDigit(peek())) advance();

    if (peek() == '.' && isDigit(peekNext())) {
        advance();
        while (isDigit(peek())) advance();
    }
    return makeToken(TOKEN_NUMBER);
}

static TokenType identifierType() {
    Node* p = root;
    int len = scanner.current - scanner.start;
    for (int i = 0; i < len; ++i) {
        if (scanner.start[i] < 'a' || scanner.start[i] > 'z')
            return TOKEN_IDENTIFIER;
        Node* child = p->children[scanner.start[i] - 'a'];
        if (child == NULL) {
            return TOKEN_IDENTIFIER;
        } else if (i == len - 1) {
            return child->tokenType != -1 ? child->tokenType : TOKEN_IDENTIFIER;
        }
        p = child;
    }
    return TOKEN_IDENTIFIER;
}

static Token identifier() {
    while (isAlpha(peek()) || isDigit(peek())) advance();
    return makeToken(identifierType());
}

Token scanToken() {
    skipWhitespace();
    scanner.start = scanner.current;
    if (isAtEnd()) return makeToken(TOKEN_EOF);

    char c = advance();
    if (isAlpha(c)) return identifier();
    if (isDigit(c)) return number();

    switch (c) {
        case '(': return makeToken(TOKEN_LEFT_PAREN);
        case ')': return makeToken(TOKEN_RIGHT_PAREN);
        case '{': return makeToken(TOKEN_LEFT_BRACE);
        case '}': return makeToken(TOKEN_RIGHT_BRACE);
        case ';': return makeToken(TOKEN_SEMICOLON);
        case ',': return makeToken(TOKEN_COMMA);
        case '.': return makeToken(TOKEN_DOT);
        case '-': return makeToken(TOKEN_MINUS);
        case '+': return makeToken(TOKEN_PLUS);
        case '/': return makeToken(TOKEN_SLASH);
        case '*': return makeToken(TOKEN_STAR);
        case '!': return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=': return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<': return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>': return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '"': return string();
    }
    return errorToken("Unexpected character.");
}