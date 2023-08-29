
#include <iostream>
#include <istream>
#include <string>
#include <cctype>
#include <cstdlib>
#include <cstring>

#include "lexer.h"

using namespace std;

string reserved[] = {"END_OF_FILE", "INT", "REAL", "BOOL", "TR", "FA", "IF", "WHILE", "SWITCH", "CASE", "PUBLIC",
                     "PRIVATE", "NUM", "REALNUM", "NOT", "PLUS", "MINUS", "MULT", "DIV", "GTEQ", "GREATER", "LTEQ",
                     "NOTEQUAL", "LESS", "LPAREN", "RPAREN", "EQUAL", "COLON", "COMMA", "SEMICOLON", "LBRACE", "RBRACE",
                     "ID", "ERROR"
};

#define KEYWORDS_COUNT 11
string keyword[] = {"int", "real", "bool", "true", "false", "if", "while", "switch", "case", "public", "private"};

LexicalAnalyzer lexer;
Token token;
TokenType tempTokenType;

struct scopeResolve {
    string scope;
    scopeResolve *next;
};

struct sTableEntry {
    string name;
    string scope;
    int pubpriv;
    int type; // INT = 1, REAL = 2, BOOL = 3, UNDECIDED > 3
};

struct sTable {
    sTableEntry *item;
    sTable *prev;
    sTable *next;

};

sTable *symbolTable;
string currentScope;
string lResolve;
string rResolve;
scopeResolve *scopeTable;
int currentPrivPub = 0;
int currentType = 4;

void addTypes(int ttype) {
    sTable *tempTable = symbolTable;
    if (tempTable == nullptr) { ;
    } else {
        while (tempTable->next != nullptr && tempTable->item->type <= 3) {
            tempTable = tempTable->next;
        }
        while (tempTable != nullptr) {
            tempTable->item->type = ttype;
            tempTable = tempTable->next;
        }
    }
}

void addScope() {

    if (scopeTable == nullptr) {

        scopeResolve *newScopeItem = (scopeResolve *) malloc(sizeof(scopeResolve));
        newScopeItem->scope = currentScope;
        newScopeItem->next = nullptr;
        scopeTable = newScopeItem;
    } else {
        scopeResolve *tempTable = scopeTable;
        while (tempTable->next != nullptr) {
            tempTable = tempTable->next;
        }

        scopeResolve *newScopeItem = (scopeResolve *) malloc(sizeof(scopeResolve));
        newScopeItem->scope = currentScope;
        newScopeItem->next = nullptr;
        tempTable->next = newScopeItem;
    }
}

void deleteScope() {
    scopeResolve *tempTable = scopeTable;
    if (tempTable != nullptr) {
        if (tempTable->next == nullptr) {
            tempTable = nullptr;
        } else {
            while (tempTable->next->next != nullptr) {
                tempTable = tempTable->next;
            }
            currentScope = tempTable->scope;
            tempTable->next = nullptr;
        }
    }

}

void addList(const string &lexeme, int tType) {

    if (symbolTable == nullptr) {
        sTable *newEntry = (sTable *) malloc(sizeof(sTable));
        sTableEntry *newItem = (sTableEntry *) malloc(sizeof(sTableEntry));

        newItem->name = lexeme;
        newItem->scope = currentScope;
        newItem->pubpriv = currentPrivPub;
        newItem->type = tType;

        newEntry->item = newItem;
        newEntry->next = nullptr;
        newEntry->prev = nullptr;

        symbolTable = newEntry;

    } else {
        sTable *temp = symbolTable;
        while (temp->next != nullptr) {
            temp = temp->next;
        }

        sTable *newEntry = (sTable *) malloc(sizeof(sTable));
        sTableEntry *newItem = (sTableEntry *) malloc(sizeof(sTableEntry));

        newItem->name = lexeme;
        newItem->scope = currentScope;
        newItem->pubpriv = currentPrivPub;
        newItem->type = tType;

        newEntry->item = newItem;
        newEntry->next = nullptr;
        newEntry->prev = temp;
        temp->next = newEntry;

    }


}

void printScope() {

    scopeResolve *temp = scopeTable;
    cout << "\n Printing Scope Table \n";
    while (temp->next != nullptr) {
        cout << " Scope " << temp->scope << " -> ";
        temp = temp->next;

    }
    cout << " Scope " << temp->scope << " \n";
}


void printList() {

    sTable *temp = symbolTable;
    cout << "\n Printing Symbol Table \n";
    while (temp->next != nullptr) {
        cout << "\n Name: " << temp->item->name << " Scope: " << temp->item->scope << " Persmission: "
             << temp->item->pubpriv << " Type: " << temp->item->type << " \n";
        temp = temp->next;

    }
    cout << "\n Name: " << temp->item->name << " Scope: " << temp->item->scope << " Persmission: "
         << temp->item->pubpriv << " Type: " << temp->item->type << " \n";
}

void deleteList() {

    sTable *temp = symbolTable;

    if (temp != nullptr) {
        while (temp->next != nullptr && (temp->item->scope != currentScope)) {
            temp = temp->next;

        }

        if (temp->item->scope == currentScope) {

            //cout << "\n found Match: " << temp->item->scope << "  " << currentScope << "\n";
            if (currentScope != "::") {
                temp = temp->prev;
                temp->next = nullptr;
            } else {
                temp = nullptr;
            }
            //deleteList();
        }
        //printList();

    }


}


void Token::Print() {
    cout << "{" << this->lexeme << " , "
         << reserved[(int) this->token_type] << " , "
         << this->line_no << "}\n";
}

LexicalAnalyzer::LexicalAnalyzer() {
    this->line_no = 1;
    tmp.lexeme = "";
    tmp.line_no = 1;
    tmp.token_type = ERROR;
}


bool LexicalAnalyzer::SkipSpace() {
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

bool LexicalAnalyzer::SkipComments() {
    bool comments = false;
    char c;
    if (input.EndOfInput()) {
        input.UngetChar(c);
        return comments;
    }

    input.GetChar(c);


    if (c == '/') {
        input.GetChar(c);
        if (c == '/') {
            comments = true;
            while (c != '\n') {
                comments = true;
                input.GetChar(c);


            }
            line_no++;

            SkipComments();
        } else {
            comments = false;
            cout << "Syntax Error\n";
            exit(0);
        }


    } else {
        input.UngetChar(c);

        return comments;
    }


}

bool LexicalAnalyzer::IsKeyword(const string &s) {
    for (const auto &i: keyword) {
        if (s == i) {
            return true;
        }
    }
    return false;
}

TokenType LexicalAnalyzer::FindKeywordIndex(const string &s) {
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return (TokenType) (i + 1);
        }
    }
    return ERROR;
}

Token LexicalAnalyzer::ScanNumber() {
    char c;
    bool realNUM = false;
    input.GetChar(c);
    if (isdigit(c)) {
        if (c == '0') {
            tmp.lexeme = "0";
            input.GetChar(c);
            if (c == '.') {

                //cout << "\n I am here too " << c << " \n";
                input.GetChar(c);

                if (!isdigit(c)) {
                    input.UngetChar(c);
                } else {
                    while (!input.EndOfInput() && isdigit(c)) {
                        tmp.lexeme += c;
                        input.GetChar(c);
                        realNUM = true;

                    }
                    input.UngetChar(c);
                }
            } else {
                input.UngetChar(c);
            }
        } else {
            tmp.lexeme = "";
            while (!input.EndOfInput() && isdigit(c)) {
                tmp.lexeme += c;
                input.GetChar(c);
            }
            if (c == '.') {

                //cout << "\n I am here too " << c << " \n";
                input.GetChar(c);

                if (!isdigit(c)) {
                    input.UngetChar(c);
                } else {
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
        if (realNUM) {
            tmp.token_type = REALNUM;
        } else {
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

Token LexicalAnalyzer::ScanIdOrKeyword() {
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

// you should unget tokens in the reverse order in which they
// are obtained. If you execute
//
//    t1 = lexer.GetToken();
//    t2 = lexer.GetToken();
//    t3 = lexer.GetToken();
//
// in this order, you should execute
//
//    lexer.UngetToken(t3);
//    lexer.UngetToken(t2);
//    lexer.UngetToken(t1);
//
// if you want to unget all three tokens. Note that it does not
// make sense to unget t1 without first ungetting t2 and t3
//
TokenType LexicalAnalyzer::UngetToken(const Token &tok) {
    tokens.push_back(tok);;
    return tok.token_type;
}

Token LexicalAnalyzer::GetToken() {
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
    //cout << "\n Char obtained " << c << "\n";
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
            if (c == '=') {
                tmp.token_type = GTEQ;
            } else {
                input.UngetChar(c);
                tmp.token_type = GREATER;
            }
            return tmp;
        case '<':
            input.GetChar(c);
            if (c == '=') {
                tmp.token_type = LTEQ;
            } else if (c == '>') {
                tmp.token_type = NOTEQUAL;
            } else {
                input.UngetChar(c);
                tmp.token_type = LESS;
            }
            return tmp;
        case '(':
            //cout << "\n I am here" << c << " \n";
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
                //cout << "\n ID scan " << c << " \n"; 
                return ScanIdOrKeyword();
            } else if (input.EndOfInput())
                tmp.token_type = END_OF_FILE;
            else
                tmp.token_type = ERROR;

            return tmp;
    }
}


// parse var_list
int parse_varlist() {
    token = lexer.GetToken();
    //token.Print();
    int tempI;

    string lexeme = (token.lexeme);
    addList(lexeme, currentType);
    /*sTable* temp2 = symbolTable;
    while(temp2!=nullptr){
        cout << "\n Symbol Table => Name: " << temp2->item->name << " Scope: " << temp2->item->scope << " Permission: " << temp2->item->pubpriv << "\n";
        if(temp2->next != nullptr){
            temp2 = temp2->next;
        }else{
            break;
        }
        
    }*/

    // Check First set of ID
    if (token.token_type == ID) {
        token = lexer.GetToken();
        //token.Print();
        if (token.token_type == COMMA) {
            cout << "\n Rule Parsed: var_list -> ID COMMA var_list \n";
            tempI = parse_varlist();
        } else if (token.token_type == COLON) {
            tempTokenType = lexer.UngetToken(token);
            cout << "\n Rule Parsed: var_list -> ID \n";


        } else {
            cout << "\n Syntax Error \n";

        }
    } else {
        cout << "\n Syntax Error \n";
    }

    return (0);

}

// parse scope



int parse_body();

Token token1;

/*int parse_stmt(){
    token = lexer.GetToken();
    //token.Print();
    int tempI;
    char* tempID;
    
    tempID = (char *)malloc((token.lexeme).length()+1);
                
    memcpy(tempID, (token.lexeme).c_str(), (token.lexeme).length()+1);
    
    if(token.token_type == ID){
      
        token1 = lexer.GetToken();
        //token1.Print();
        if(token1.token_type == LBRACE){
            tempTokenType = lexer.UngetToken(token1);
            tempTokenType = lexer.UngetToken(token);
            //cout << "\n Rule Parsed: stmt -> scope\n";
            tempI = parse_scope();
            
        }else if(token1.token_type == EQUAL){
            token = lexer.GetToken();
            //token.Print();
            if(token.token_type == ID){
                
                searchList(tempID,0);
                if(strcmp(lResolve,"::") == 0){
                
                    cout << lResolve << tempID << " = ";
                }else{
                    cout << lResolve << "." << tempID << " = ";
                }
                free(tempID);
                free(lResolve);
                
                tempID = (char *)malloc((token.lexeme).length()+1);
                
                memcpy(tempID, (token.lexeme).c_str(), (token.lexeme).length()+1);
                
                searchList(tempID,1);
                //cout<< "\n tempID " << tempID << "\n";
                if(strcmp(rResolve,"::") == 0){
                
                    cout << rResolve << tempID << "\n";
                }else{
                    cout << rResolve << "." << tempID << "\n";
                }
                free(tempID);
                
                free(rResolve);
                token = lexer.GetToken();
                //token.Print();
                if(token.token_type == SEMICOLON){
                    //cout << "\n Rule Parsed: stmt -> ID EQUAL ID SEMICOLON \n";
                    
                }else{
                    cout << "\n Syntax Error \n";
                }
            }else{
                cout << "\n Syntax Error \n";
            }
        }else{
            cout << "\n Syntax Error \n";
        }
    
    }else{
        cout << "\n Syntax Error \n";
    }
    return(0);

}*/
int sanatizeNumberType(int ttype) {
    if (ttype == NUM) {
        return INT;
    }
    if (ttype == REALNUM) {
        return REAL;
    }
    if (ttype == TR || ttype == FA || ttype == GTEQ) {
        return BOOL;
    }
}

int parse_unaryOperator() {
    token = lexer.GetToken();

    if (token.token_type == NOT) {
        cout << "\n Rule parsed: unary_operator -> NOT";
    } else {
        cout << "\n Syntax Error\n";
    }

    return (0);

}


int parse_binaryOperator() {
    token = lexer.GetToken();
    if (token.token_type == PLUS) {

        cout << "\n Rule parsed: binary_operator -> PLUS\n";
    } else if (token.token_type == MINUS) {

        cout << "\n Rule parsed: binary_operator -> MINUS \n";

    } else if (token.token_type == MULT) {
        cout << "\n Rule parsed: binary_operator -> MULT\n";

    } else if (token.token_type == DIV) {

        cout << "\n Rule parsed: binary_operator -> DIV \n";

    } else if (token.token_type == GREATER) {
        cout << "\n Rule parsed: binary_operator -> GREATER \n";
    } else if (token.token_type == LESS) {

        cout << "\n Rule parsed: binary_operator -> LESS\n";
    } else if (token.token_type == GTEQ) {

        cout << "\n Rule parsed: binary_operator -> GTEQ \n";

    } else if (token.token_type == LTEQ) {
        cout << "\n Rule parsed: binary_operator -> LTEQ\n";

    } else if (token.token_type == EQUAL) {

        cout << "\n Rule parsed: binary_operator -> EQUAL \n";

    } else if (token.token_type == NOTEQUAL) {
        cout << "\n Rule parsed: binary_operator -> NOTEQUAL \n";
    } else {
        cout << "\n Syntax Error \n";
    }
    return (0);

}


int parse_primary() {
    token = lexer.GetToken();

    if (token.token_type == ID) {

        cout << "\n Rule parsed: primary -> ID\n";
    } else if (token.token_type == NUM) {

        cout << "\n Rule parsed: primary -> NUM \n";

    } else if (token.token_type == REALNUM) {
        cout << "\n Rule parsed: primary -> REALNUM\n";

    } else if (token.token_type == TR) {

        cout << "\n Rule parsed: primary -> TRUE \n";

    } else if (token.token_type == FA) {
        cout << "\n Rule parsed: primary -> FALSE \n";
    } else {
        cout << "\n Syntax Error \n";
    }
    return (0);

}


int parse_expression() {
    int tempI;

    token = lexer.GetToken();

    if (token.token_type == ID || token.token_type == NUM || token.token_type == REALNUM || token.token_type == TR ||
        token.token_type == FA) {

        cout << "\n Rule parsed: expression -> primary \n";
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_primary();


    } else if (token.token_type == PLUS || token.token_type == MINUS || token.token_type == MULT ||
               token.token_type == DIV || token.token_type == GREATER || token.token_type == LESS ||
               token.token_type == GTEQ || token.token_type == LTEQ || token.token_type == EQUAL ||
               token.token_type == NOTEQUAL) {
        cout << "\n Rule parsed: expression -> binary_operator expression expression \n";
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_binaryOperator();
        tempI = parse_expression();
        tempI = parse_expression();

    } else if (token.token_type == NOT) {
        cout << "\n Rule parsed: expression -> unary_operator expression \n";
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_unaryOperator();
        tempI = parse_expression();

    } else {
        cout << "\n Syntax Error \n";
    }
    return (0);

}

sTableEntry *searchList(const string &iD, int lR) {
    bool found = false;

    sTable *temp = symbolTable;
    //cout << "\n I am here " << currentScope;// << temp->item->name << "  " << iD << " \n";

    if (temp == nullptr) {
        if (lR == 0) {
            lResolve = "?";
        } else {
            rResolve = "?";
        }

    } else {
        int count = 0;
        while (temp->next != nullptr) {
            temp = temp->next;
            count++;
        }

        //

        if (temp->item->name == iD) {
            if (temp->item->scope == currentScope) {
                found = true;
                if (lR == 0) {
                    lResolve = currentScope;
                } else {
                    rResolve = currentScope;
                }

            } else if (temp->item->pubpriv == 0) {
                found = true;
                if (lR == 0) {
                    lResolve = temp->item->scope;
                } else {
                    rResolve = temp->item->scope;
                }

            } else {
                found = false;
                //temp = temp->prev;

            }


        }


        ///

        while ((temp->item->name != iD) || !found) {

            if (temp->prev == nullptr && (temp->item->name != iD)) {

                if (lR == 0) {
                    lResolve = "?";
                } else {
                    //cout << "\n I am here " << temp->item->name << "  " << iD << " \n";
                    rResolve = "?";
                }
                found = false;

                break;
            } else {
                found = true;
            }
            if (temp->item->name == iD) {
                if (temp->item->scope == currentScope) {
                    found = true;
                    if (lR == 0) {
                        lResolve = currentScope;
                    } else {
                        rResolve = currentScope;
                    }
                    break;
                } else if (temp->item->pubpriv == 0) {
                    found = true;
                    if (lR == 0) {
                        lResolve = temp->item->scope;
                    } else {
                        rResolve = temp->item->scope;
                    }
                    break;
                } else {
                    found = false;
                    temp = temp->prev;

                }


            } else {
                found = false;
                temp = temp->prev;

            }


        }


        //   cout << "\n ID found " << iD << "\n";
    }
    return found ? temp->item : nullptr;


}

TokenType getLiteralType(TokenType tokenType) {
    TokenType type = ERROR;
    if (tokenType == NUM) {
        type = INT;
    } else if (tokenType == REALNUM) {
        type = REAL;
    } else if (tokenType == TR || tokenType == FA) {
        type = BOOL;
    }
    return type;
}

void check_type_binaryOP(TokenType lhOfAs, bool isAssignment) {
    Token op = lexer.GetToken();
    Token arg1 = lexer.GetToken();
    Token arg2 = lexer.GetToken();
    TokenType lht = ERROR;
    if (arg1.token_type == ID) {
        string lex = arg1.lexeme;
        //Left hand type
        sTableEntry* t = searchList(lex, 0);
        if (t != nullptr) {
            lht = static_cast<TokenType>(t->type);
        }
    }
    else {
        string lex = arg1.lexeme;
        //Left hand type
        lht = getLiteralType(arg1.token_type);
    }
    TokenType rht = ERROR;
    if (arg2.token_type == ID) {
        string lex = arg2.lexeme;
        //Left hand type
        sTableEntry* t = searchList(lex, 1);
        if (t != nullptr) {
            rht = static_cast<TokenType>(t->type);
        }
    }
    else {
        string lex = arg2.lexeme;
        //Left hand type
        rht = getLiteralType(arg2.token_type);
    }
    if (lht != ERROR && rht != ERROR) {
        if (lht != rht) {
            cout << "\n TYPE MISMATCH " << op.line_no << " C2 \n";
            exit(0);
        }
    } else {
        // Variable Not declared yet.
        cout << "\n Syntax Error Undefined Variable at " << op.line_no << "\n";
    }
    TokenType rhOfAs = ERROR;
    if (lhOfAs != ERROR) {
        //    The type of expression binary_operator op1 op2 is bool if operator is GREATER, LESS,
        //    GTEQ, LTEQ, EQUAL, or NOTEQUAL.
        if (op.token_type == GREATER || op.token_type == LESS || op.token_type == GTEQ || op.token_type == LTEQ ||
            op.token_type == EQUAL || op.token_type == NOTEQUAL) {
            rhOfAs = BOOL;
        } else if (op.token_type == PLUS || op.token_type == MINUS || op.token_type == MULT || op.token_type == DIV) {
            rhOfAs = lht;
        }
        if (rhOfAs != lhOfAs) {
            cout << "\n TYPE MISMATCH " << op.line_no << " C1 \n";
            exit(0);
        }
    }
    else if (isAssignment && rhOfAs != ERROR) {
        // Variable type is not defined yet.
        sTableEntry* t = searchList(token.lexeme, 0);
        if (t != nullptr) {
            t->type = static_cast<int>(rhOfAs);
        }
    }
    lexer.UngetToken(arg2);
    lexer.UngetToken(arg1);
    lexer.UngetToken(op);
}

void check_type_unaryOP() {
    Token op = lexer.GetToken();
    Token arg1 = lexer.GetToken();
    TokenType lht = ERROR;
    if (arg1.token_type == ID) {
        string lex = arg1.lexeme;
        //Left hand type
        sTableEntry* t = searchList(lex, 0);
        if (t != nullptr) {
            lht = static_cast<TokenType>(t->type);
        }
    }
    else {
        string lex = arg1.lexeme;
        //Left hand type
        lht = getLiteralType(arg1.token_type);
    }
    if (lht != ERROR) {
        if (lht != BOOL) {
            cout << "\n TYPE MISMATCH " << op.line_no << " C3 \n";
            exit(0);
        }
    } else {
        // Variable Not declared yet.
        cout << "\n Syntax Error Undefined Variable at " << op.line_no << "\n";
    }
    lexer.UngetToken(arg1);
    lexer.UngetToken(op);
}

void check_type_OPS(TokenType lht = ERROR, bool isAssignment = false) { //lht = ERROR means no type defined yet for left hand side of assignment
    Token op = lexer.GetToken();
    if (op.token_type == PLUS || op.token_type == MINUS || op.token_type == MULT || op.token_type == DIV ||
        op.token_type == GREATER || op.token_type == LESS || op.token_type == GTEQ || op.token_type == LTEQ ||
        op.token_type == EQUAL || op.token_type == NOTEQUAL) {
        lexer.UngetToken(op);
        check_type_binaryOP(lht, isAssignment);
    } else if (op.token_type == NOT) {
        lexer.UngetToken(op);
        check_type_unaryOP();
    }
    lexer.UngetToken(op);
}
int parse_assstmt() {
    int tempI;
    token = lexer.GetToken();
    //cout << "\n token name " << token.lexeme << " \n";
    if (token.token_type == ID) {
        TokenType lht = token.token_type;
        sTableEntry* t = searchList(token.lexeme, 0);
        if (t != nullptr) {
            lht = static_cast<TokenType>(t->type);
        }
        else {
            addList(token.lexeme, 4);
            lht = ERROR;
        }
        Token op = lexer.GetToken();
        //cout << "\n token name " << token.lexeme << " \n";
        if (op.token_type == EQUAL) {
            check_type_OPS(lht, true);
            token = lexer.GetToken();
            // Find right hand side type
            t = searchList(token.lexeme, 0);
            if (t == nullptr) {
                addList(token.lexeme, 4);
            }
            //cout << "\n token name " << token.lexeme << " \n";
            if (token.token_type == ID || token.token_type == NUM || token.token_type == REALNUM ||
                token.token_type == TR || token.token_type == FA || token.token_type == PLUS ||
                token.token_type == MINUS || token.token_type == MULT || token.token_type == DIV ||
                token.token_type == LESS || token.token_type == GREATER || token.token_type == GTEQ ||
                token.token_type == LTEQ || token.token_type == EQUAL || token.token_type == NOTEQUAL ||
                token.token_type == NOT) {
                tempTokenType = lexer.UngetToken(token);
                tempI = parse_expression();
                token = lexer.GetToken();
                //token.Print();
                if (token.token_type == SEMICOLON) {
                    cout << "\n Rule parsed: assignment_stmt -> ID EQUAL expression SEMICOLON \n";

                } else {
                    cout << "\n HI Syntax Error " << token.token_type << " \n";
                }
            } else {
                cout << "\n Syntax Error \n";
            }
        } else {
            cout << "\n Syntax Error \n";
        }

    } else {
        cout << "\n Syntax Error \n";
    }
    return (0);
}

int parse_case() {

    int tempI;
    token = lexer.GetToken();
    if (token.token_type == CASE) {
        token = lexer.GetToken();
        if (token.token_type == NUM) {
            token = lexer.GetToken();
            if (token.token_type == COLON) {
                cout << "\n Rule parsed: case -> CASE NUM COLON body";
                tempI = parse_body();
            } else {
                cout << "\n Syntax Error \n";
            }

        } else {
            cout << "\n Syntax Error \n";
        }

    } else {
        cout << "\n Syntax Error \n";
    }
}

int parse_caselist() {

    int tempI;
    token = lexer.GetToken();
    if (token.token_type == CASE) {
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_case();
        token = lexer.GetToken();
        if (token.token_type == CASE) {
            tempTokenType = lexer.UngetToken(token);
            cout << "\n Rule parsed: case_list -> case case_list \n";
            tempI = parse_caselist();
        } else if (token.token_type == RBRACE) {
            tempTokenType = lexer.UngetToken(token);
            cout << "\n Rule parsed: case_list -> case  \n";
        }
    }
    return (0);
}


int parse_switchstmt() {
    int tempI;

    token = lexer.GetToken();
    if (token.token_type == SWITCH) {
        token = lexer.GetToken();
        if (token.token_type == LPAREN) {
            tempI = parse_expression();
            token = lexer.GetToken();
            if (token.token_type == RPAREN) {
                token = lexer.GetToken();
                if (token.token_type == LBRACE) {
                    tempI = parse_caselist();
                    token = lexer.GetToken();
                    if (token.token_type == RBRACE) {
                        cout
                                << "\n Rule parsed: switch_stmt -> SWITCH LPAREN expression RPAREN LBRACE case_list RBRACE \n";
                    } else {
                        cout << "\n Syntax Error \n";
                    }
                } else {
                    cout << "\n Syntax Error \n";
                }

            } else {
                cout << "\n Syntax Error \n";
            }
        } else {
            cout << "\n Syntax Error \n";
        }
    } else {
        cout << "\n Syntax Error \n";
    }
    return (0);
}


int parse_whilestmt() {
    int tempI;

    token = lexer.GetToken();
    if (token.token_type == WHILE) {
        token = lexer.GetToken();
        if (token.token_type == LPAREN) {
            tempI = parse_expression();
            token = lexer.GetToken();
            if (token.token_type == RPAREN) {
                cout << "\n Rule parsed: whilestmt -> WHILE LPAREN expression RPAREN body \n";
                tempI = parse_body();

            } else {
                cout << "\n Syntax Error \n";
            }
        } else {
            cout << "\n Syntax Error \n";
        }
    } else {
        cout << "\n Syntax Error \n";
    }
    return (0);
}

int parse_ifstmt() {
    int tempI;

    token = lexer.GetToken();
    if (token.token_type == IF) {
        token = lexer.GetToken();
        if (token.token_type == LPAREN) {
            tempI = parse_expression();
            token = lexer.GetToken();
            if (token.token_type == RPAREN) {
                cout << "\n Rule parsed: ifstmt -> IF LPAREN expression RPAREN body \n";
                tempI = parse_body();

            } else {
                cout << "\n Syntax Error \n";
            }
        } else {
            cout << "\n Syntax Error \n";
        }
    } else {
        cout << "\n Syntax Error \n";
    }
    return (0);
}

int parse_stmt() {
    int tempI;
    token = lexer.GetToken();
    if (token.token_type == ID) {
        tempTokenType = lexer.UngetToken(token);
        cout << "\n Rule parsed: stmt -> assignment_stmt \n";
        tempI = parse_assstmt();

    } else if (token.token_type == IF) {
        tempTokenType = lexer.UngetToken(token);
        cout << "\n Rule parsed: stmt -> if_stmt";
        tempI = parse_ifstmt();
    } else if (token.token_type == WHILE) {
        tempTokenType = lexer.UngetToken(token);
        cout << "\n Rule parsed: stmt -> while_stmt";
        tempI = parse_whilestmt();
    } else if (token.token_type == SWITCH) {
        tempTokenType = lexer.UngetToken(token);
        cout << "\n Rule parsed: stmt -> switch_stmt";
        tempI = parse_switchstmt();
    } else {
        cout << "\n Syntax Error \n";
    }
    return (0);
}

int parse_stmtlist() {

    token = lexer.GetToken();
    //token.Print();
    int tempI;
    if (token.token_type == ID || token.token_type == IF || token.token_type == WHILE || token.token_type == SWITCH) {
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_stmt();
        token = lexer.GetToken();
        //token.Print();
        if (token.token_type == ID || token.token_type == IF || token.token_type == WHILE ||
            token.token_type == SWITCH) {
            tempTokenType = lexer.UngetToken(token);
            cout << "\n Rule Parsed: stmt_list -> stmt stmt_list \n";
            tempI = parse_stmtlist();

        } else if (token.token_type == RBRACE) {
            tempTokenType = lexer.UngetToken(token);
            //printList();
            cout << "\n Rule parsed: stmt_list -> stmt \n";
        }
    } else {
        cout << "\n Syntax Error \n";

    }
    return (0);
}


int parse_body() {

    token = lexer.GetToken();
    //token.Print();
    int tempI;
    // currentScope = (char*)malloc(sizeof(token.lexeme)+1);
    //memcpy(currentScope, (token.lexeme).c_str(), (token.lexeme).size()+1);
    // addScope();
    //strcpy(currentScope, lexeme);

    if (token.token_type == LBRACE) {
        //cout << "\n Rule Parsed: scope -> ID LBRACE public_vars private_vars stmt_list RBRACE \n";

        tempI = parse_stmtlist();
        token = lexer.GetToken();
        //token.Print();
        if (token.token_type == RBRACE) {
            //if(symbolTable != nullptr){
            //    deleteList();


            //}
            //printScope();
            //deleteScope();
            cout << "\n Rule parsed: body -> LBRACE stmt_list RBRACE \n";
            return (0);
        } else {
            cout << "\n Syntax Error\n ";
            return (0);
        }


    } else if (token.token_type == END_OF_FILE) {
        tempTokenType = lexer.UngetToken(token);
        return (0);
    } else {
        cout << "\n Syntax Error \n ";
        return (0);
    }

}




// parse scope end

int parse_typename() {
    token = lexer.GetToken();
    if (token.token_type == INT || token.token_type == REAL || token.token_type == BOOL) {
        cout << "\n Rule parse: type_name -> " << token.token_type << "\n";
        return token.token_type;
    } else {
        cout << "\n Syntax Error \n";
    }
    return (0);
}

int parse_vardecl() {
    int tempI;
    token = lexer.GetToken();
    if (token.token_type == ID) {
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_varlist();
        Token token2 = lexer.GetToken();
        if (token2.token_type == COLON) {
            tempI = parse_typename();
            addTypes(tempI);
            token = lexer.GetToken();
            if (token.token_type == SEMICOLON) {
                cout << "\n Rule parsed: var_decl -> var_list COLON type_name SEMICOLON";
            } else {
                cout << "\n Syntax Error \n";
            }
        } else {
            cout << "\n Syntax Error \n";
        }
    } else {
        cout << "\n Syntax Error \n";

    }
    return (0);
}

int parse_vardecllist() {
    int tempI;
    token = lexer.GetToken();
    while (token.token_type == ID) {
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_vardecl();
        token = lexer.GetToken();
        if (token.token_type != ID) {
            cout << "\n Rule Parsed: var_decl_list -> var_decl \n";

        } else {
            cout << "\n Rule Parsed: var_decl_list -> var_decl var_decl_list \n";
        }
    }
    tempTokenType = lexer.UngetToken(token);
    return (0);
}


string global = "::";

// parse global vars
int parse_globalVars() {
    token = lexer.GetToken();
    //token.Print();
    int tempI;

    //check first set of var_list SEMICOLON
    if (token.token_type == ID) {
        tempTokenType = lexer.UngetToken(token);
        currentPrivPub = 0;
        //strcpy(currentScope,global);
        cout << "\n Rule parsed: globalVars -> var_decl_list \n";
        tempI = parse_vardecllist();


    } else {
        cout << "Syntax Error";
    }
    return (0);

}


int parse_program() {

    token = lexer.GetToken();
    //token.Print();
    int tempI;
    while (token.token_type != END_OF_FILE) {
        // Check first set of global_vars scope
        if (token.token_type == ID) {
            tempTokenType = lexer.UngetToken(token);
            //token1.Print();
            cout << "\n Rule parsed: program -> global_vars scope \n";
            tempI = parse_globalVars();
            tempI = parse_body();

        } else if (token.token_type == LBRACE) {
            tempTokenType = lexer.UngetToken(token);

            cout << "\n Rule parsed: global_vars -> epsilon \n";
            tempI = parse_body();


        } else if (token.token_type == END_OF_FILE) {
            return (0);
        } else {
            cout << "\n Syntax Error\n";
            return (0);
        }
        token = lexer.GetToken();
        //token.Print();
    }


}

char null[] = "nullptr";
void noTypeError() {
    sTable *temp = symbolTable;
    while (temp != nullptr) {
        if (temp->item->type == 1) {
            cout << temp->item->name << ": " << "int #" << "\n";
        }
        else if (temp->item->type == 2) {
            cout << temp->item->name << ": " << "real #" << "\n";
        }
        else if (temp->item->type == 3) {
            cout << temp->item->name << ": " << "bool #" << "\n";
        }
        else {
            cout << temp->item->name << ": " << "? #" << "\n";
        }
        temp = temp->next;
    }
}
int main() {
    //symbolTable = (sTable *) malloc(sizeof(sTable));ass
    //symbolTable = nullptr;
    //scopeTable = nullptr;
    //currentScope = (char *)malloc(sizeof(global)+1);
    //memcpy(currentScope,global.c_str(),global.length()+1);
    //addScope();
    int i = parse_program();
    // NO TYPE ERROR
    noTypeError();
    cout << i << "\n End of Program \n";
    //printList();
    //free(symbolTable);
    //token = lexer.GetToken();
    //cout << "\n Token value " << token.lexeme << "\n";
    //token.Print();
    //while(token.token_type != END_OF_FILE){
    //cout << "\n Token value " << token.token_type << "\n";
    //   token = lexer.GetToken();
    //cout << "\n Token Type " << token.token_type << " \n";
    // token.Print();
    //}
}