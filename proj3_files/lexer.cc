#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <cctype>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "inputbuf.h"

using namespace std;

string reserved[] = { "END_OF_FILE", "INT", "REAL", "BOOL", "TR", "FA", "IF", "WHILE", "SWITCH", "CASE", "PUBLIC", "PRIVATE", "NUM", "REALNUM", "NOT", "PLUS", "MINUS", "MULT", "DIV", "GTEQ", "GREATER", "LTEQ", "NOTEQUAL", "LESS", "LPAREN", "RPAREN", "EQUAL", "COLON", "COMMA", "SEMICOLON", "LBRACE", "RBRACE", "ID", "ERROR" };

#define KEYWORDS_COUNT 11
string keyword[] = { "int", "real", "bool", "true", "false", "if", "while", "switch", "case", "public", "private" };

LexicalAnalyzer::LexicalAnalyzer()
{
    this->line_no = 1;
    tmp.lexeme = "";
    tmp.line_no = 1;
    tmp.token_type = ERROR;
}

bool LexicalAnalyzer::SkipSpace()
{
    char c;
    bool space_encountered = false;

    input.GetChar(c);
    line_no += (c == '\n');

    while (!input.EndOfInput() && isspace(c)) {
        space_encountered = true;
        input.GetChar(c);
        line_no += (c == '\n');
    }

    if (!input.EndOfInput()) {
        input.UngetChar(c);
    }
    return space_encountered;
}

bool LexicalAnalyzer::SkipComments()
{
    bool comments = false;
    char c;
    if(input.EndOfInput() ){
        input.UngetChar(c);
        return comments;
    }
    
    input.GetChar(c);
    
    if(c == '/'){
        input.GetChar(c);
        if(c == '/'){
            comments = true;
            while(c != '\n'){
                comments = true;
                input.GetChar(c);
            }
            line_no++;
            SkipSpace();
            SkipComments();
        }else{
            comments = false;
            input.UngetChar(c);
            input.UngetChar('/');
        }
    }else{
           input.UngetChar(c);
           comments = false;
    }
    return comments;
}

bool LexicalAnalyzer::IsKeyword(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return true;
        }
    }
    return false;
}

TokenType LexicalAnalyzer::FindKeywordIndex(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return (TokenType) (i + 1);
        }
    }
    return ERROR;
}

Token LexicalAnalyzer::ScanNumber()
{
    char c;
    bool realNUM = false;
    input.GetChar(c);
    if (isdigit(c)) {
        if (c == '0') {
            tmp.lexeme = "0";
            input.GetChar(c);
            if(c == '.'){
                input.GetChar(c);
                
                if(!isdigit(c)){
                    input.UngetChar(c);
                }else{
                    while (!input.EndOfInput() && isdigit(c)) {
                        tmp.lexeme += c;
                        input.GetChar(c);
                        realNUM = true;
                    }   
                    input.UngetChar(c);
                }
            }else{
                input.UngetChar(c);
            }
        } else {
            tmp.lexeme = "";
            while (!input.EndOfInput() && isdigit(c)) {
                tmp.lexeme += c;
                input.GetChar(c);
            }
            if(c == '.'){
                input.GetChar(c);
                
                if(!isdigit(c)){
                    input.UngetChar(c);
                }else{
                    while (!input.EndOfInput() && isdigit(c)) {
                        tmp.lexeme += c;
                        input.GetChar(c);
                        realNUM = true;
                    }   
                }
            }
            if (!input.EndOfInput()) {
                input.UngetChar(c);
            }
        }
        if(realNUM){
            tmp.token_type = REALNUM;
        }else{
            tmp.token_type = NUM;
        }
        tmp.line_no = line_no;
        return tmp;
    } else { 
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
        tmp.line_no = line_no;
        return tmp;
    }
}

Token LexicalAnalyzer::ScanIdOrKeyword()
{
    char c;
    input.GetChar(c);
    if (isalpha(c)) {
        tmp.lexeme = "";
        while (!input.EndOfInput() && isalnum(c)) {
            tmp.lexeme += c;
            input.GetChar(c);
        }
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.line_no = line_no;
        if (IsKeyword(tmp.lexeme))
            tmp.token_type = FindKeywordIndex(tmp.lexeme);
        else
            tmp.token_type = ID;
    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
    }
    return tmp;
}

TokenType LexicalAnalyzer::UngetToken(Token tok)
{
    tokens.push_back(tok);;
    return tok.token_type;
}

Token LexicalAnalyzer::GetToken()
{
    char c;
    // if there are tokens that were previously
    // stored due to UngetToken(), pop a token and
    // return it without reading from input
    if (!tokens.empty()) {
        tmp = tokens.back();
        tokens.pop_back();
        return tmp;
    }
    SkipSpace();
    SkipComments();
    SkipSpace();
    tmp.lexeme = "";
    tmp.line_no = line_no;
    input.GetChar(c);
    switch (c) {
        case '!':
            tmp.token_type = NOT;
            return tmp;    
        case '+':
            tmp.token_type = PLUS;
            return tmp;    
        case '-':
            tmp.token_type = MINUS;
            return tmp; 
        case '*':
            tmp.token_type = MULT;
            return tmp;       
        case '/':
            tmp.token_type = DIV;
            return tmp;    
        case '>':
            input.GetChar(c);
            if(c == '='){
                tmp.token_type = GTEQ;   
            }else{
                input.UngetChar(c);
                tmp.token_type = GREATER;
            }
            return tmp;    
        case '<':
            input.GetChar(c);
            if(c == '='){
                tmp.token_type = LTEQ;   
            }else if (c == '>'){
                tmp.token_type = NOTEQUAL;    
            }else{
                input.UngetChar(c);
                tmp.token_type = LESS;
            }
            return tmp;            
        case '(':
            tmp.token_type = LPAREN;
            return tmp;    
        case ')':
            tmp.token_type = RPAREN;
            return tmp;    
        case '=':
            tmp.token_type = EQUAL;
            return tmp;
        case ':':
            tmp.token_type = COLON;
            return tmp;
        case ',':
            tmp.token_type = COMMA;
            return tmp;
        case ';':
            tmp.token_type = SEMICOLON;
            return tmp;
        case '{':
            tmp.token_type = LBRACE;
            return tmp;
        case '}':
            tmp.token_type = RBRACE;
            return tmp;
        default:
            if (isdigit(c)) {
                input.UngetChar(c);
                return ScanNumber();
            } else if (isalpha(c)) {
                input.UngetChar(c);
                return ScanIdOrKeyword();
            } else if (input.EndOfInput())
                tmp.token_type = END_OF_FILE;
            else
                tmp.token_type = ERROR;
            return tmp;
    }
}
