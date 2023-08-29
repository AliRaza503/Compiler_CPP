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

string reserved[] = { "END_OF_FILE", "INT", "REAL", "BOO", "TR", "FA", "IF", "WHILE", "SWITCH", "CASE", "PUBLIC", "PRIVATE", "NUM", "REALNUM", "NOT", "PLUS", "MINUS", "MULT", "DIV", "GTEQ", "GREATER", "LTEQ", "NOTEQUAL", "LESS", "LPAREN", "RPAREN", "EQUAL", "COLON", "COMMA", "SEMICOLON", "LBRACE", "RBRACE", "ID", "ERROR" // TODO: Add labels for new token types here (as string)
};

#define KEYWORDS_COUNT 11
string keyword[] = { "int", "real", "bool", "true", "false", "if", "while", "switch", "case", "public", "private" };

    LexicalAnalyzer lexer;
    Token token;
    TokenType tempTokenType;

struct scopeResolve {
 char* scope;
 scopeResolve* next;
};

struct sTableEntry {
    char* name;
    char* scope;
    int pubpriv;
    // int binNo;

};

struct sTable {
sTableEntry* item;
sTable *prev;
sTable *next;

};

sTable* symbolTable;
char* currentScope;
char* lResolve;
char* rResolve;
scopeResolve* scopeTable;
int currentPrivPub = 0;

void addScope(void){
    
    if(scopeTable == NULL){
        
        scopeResolve* newScopeItem = (scopeResolve *) malloc(sizeof(scopeResolve));
        newScopeItem->scope = (char *)malloc(sizeof(currentScope));
        memcpy(newScopeItem->scope,currentScope,sizeof(currentScope));
        newScopeItem->next = NULL;
        scopeTable = newScopeItem;
    }else{
        scopeResolve* tempTable = scopeTable;
        while(tempTable->next != NULL){
            tempTable = tempTable->next;
        }   
        
        scopeResolve* newScopeItem = (scopeResolve *) malloc(sizeof(scopeResolve));
        newScopeItem->scope = (char *)malloc(sizeof(currentScope));
        memcpy(newScopeItem->scope,currentScope,sizeof(currentScope));
        newScopeItem->next = NULL;
        tempTable->next = newScopeItem;
    }
}

void deleteScope(void){
        scopeResolve* tempTable = scopeTable;
        if(tempTable != NULL){
            if(tempTable->next == NULL){
                tempTable = NULL;
            }else{
                while(tempTable->next->next != NULL){
                    tempTable = tempTable->next;
                }
                currentScope = (char *)malloc(sizeof(tempTable->scope));
                memcpy(currentScope,tempTable->scope,sizeof(tempTable->scope));
                tempTable->next = NULL;   
            }
        }

}

void addList(char* lexeme){

if(symbolTable == NULL){
    sTable* newEntry = (sTable *) malloc(sizeof(sTable));
    sTableEntry* newItem = (sTableEntry *)malloc(sizeof(sTableEntry));
    
    newItem->name = lexeme;
    newItem->scope = currentScope;
    //memcpy(newItem->scope, currentScope.c_str(), currentScope.size()+1);
    newItem->pubpriv = currentPrivPub;
    
    newEntry->item = newItem;
    newEntry->next = NULL;
    newEntry->prev = NULL;
    
    symbolTable = newEntry;
    
}else{
    sTable* temp = symbolTable;
    while(temp->next != NULL){
        temp = temp->next;
    }
    
    sTable* newEntry = (sTable *) malloc(sizeof(sTable));
    sTableEntry* newItem = (sTableEntry *)malloc(sizeof(sTableEntry));
    
    newItem->name = lexeme;
    newItem->scope = currentScope;
    //memcpy(newItem->scope, currentScope.c_str(), currentScope.size()+1);
    newItem->pubpriv = currentPrivPub;
    
    newEntry->item = newItem;
    newEntry->next = NULL;
    newEntry->prev = temp;
    temp->next = newEntry;

}



}

void printScope(void){
    
    scopeResolve* temp = scopeTable;
    cout << "\n Printing Scope Table \n";
    while(temp->next != NULL){
        cout << " Scope " << temp->scope << " -> ";
        temp = temp->next;
        
    }
    cout << " Scope " << temp->scope << " \n";
}


void printList(void){
    
    sTable* temp = symbolTable;
    cout << "\n Printing Symbol Table \n";
    while(temp->next != NULL){
        cout << "\n Name: " << temp->item->name << " Scope: " << temp->item->scope << " Persmission: " << temp->item->pubpriv << " \n";
        temp = temp->next;
        
    }
    cout << "\n Name: " << temp->item->name << " Scope: " << temp->item->scope << " Persmission: " << temp->item->pubpriv << " \n";
}

void deleteList(void){
    
    sTable* temp = symbolTable;
    
    if(temp!= NULL){
        while(temp->next != NULL && strcmp(temp->item->scope,currentScope) != 0){
            temp = temp->next;
            
        }
        
        if(strcmp(temp->item->scope,currentScope) == 0){
            
            //cout << "\n found Match: " << temp->item->scope << "  " << currentScope << "\n";
            if(strcmp(currentScope,"::") != 0){
            temp = temp->prev;
            temp->next = NULL;
            }else{
                temp = NULL;
            }
            //deleteList();
        }
        //printList();
    
    }
    

}

void searchList(char* iD, int lR){// add an argument to accept a type
    bool found = false;
    
    sTable* temp = symbolTable;
    //cout << "\n I am here " << currentScope;// << temp->item->name << "  " << iD << " \n";
    
    if(temp == NULL){
        if(lR == 0){
            lResolve = (char *) malloc(1);
            memcpy(lResolve, "?", 1);
        }else{
            rResolve = (char *) malloc(1);
            memcpy(rResolve, "?",1);
        }
        
    }else{
        int count = 0;
        while(temp->next != NULL){
            temp = temp->next;
            count++;
        }
        
        //
        
        if(strcmp(temp->item->name, iD) == 0){
                if(strcmp(temp->item->scope, currentScope) == 0 ){
                    found = true;
                    if(lR == 0){
                        lResolve = (char *) malloc(sizeof(currentScope));
                        memcpy(lResolve, currentScope, sizeof(currentScope));
                    }else{
                        rResolve = (char *) malloc(sizeof(currentScope));
                        memcpy(rResolve, currentScope, sizeof(currentScope));
                    }
                    
                }else if(temp->item->pubpriv == 0){
                    found = true;
                    if(lR == 0){
                        lResolve = (char *) malloc(sizeof(temp->item->scope));
                        memcpy(lResolve, temp->item->scope, sizeof(temp->item->scope));
                    }else{
                        rResolve = (char *) malloc(sizeof(temp->item->scope));
                        memcpy(rResolve, temp->item->scope, sizeof(temp->item->scope));
                    }
                    
                }else{
                    found = false;
                    //temp = temp->prev;
                    
                }
                
                
                
            
            }
        
        
        ///

        while(strcmp(temp->item->name, iD) != 0 || !found){
            
            if(temp->prev == NULL && strcmp(temp->item->name, iD) != 0){
                
                if(lR == 0){
                    lResolve = (char *) malloc(1);
                    memcpy(lResolve, "?", 1);
                }else{
                    //cout << "\n I am here " << temp->item->name << "  " << iD << " \n";
                    rResolve = (char *) malloc(1);
                    memcpy(rResolve, "?", 1);
                }
                found = false;
                
                break;
            }else{
                found = true;
            }
            if(strcmp(temp->item->name, iD) == 0){
                if(strcmp(temp->item->scope, currentScope) == 0 ){
                    found = true;
                    if(lR == 0){
                        lResolve = (char *) malloc(sizeof(currentScope));
                        memcpy(lResolve, currentScope, sizeof(currentScope));
                    }else{
                        rResolve = (char *) malloc(sizeof(currentScope));
                        memcpy(rResolve, currentScope, sizeof(currentScope));
                    }
                    break;
                }else if(temp->item->pubpriv == 0){
                    found = true;
                    if(lR == 0){
                        lResolve = (char *) malloc(sizeof(temp->item->scope));
                        memcpy(lResolve, temp->item->scope, sizeof(temp->item->scope));
                    }else{
                        rResolve = (char *) malloc(sizeof(temp->item->scope));
                        memcpy(rResolve, temp->item->scope, sizeof(temp->item->scope));
                    }
                    break;
                }else{
                    found = false;
                    temp = temp->prev;
                    
                }
                
                
                
            
            }else{
                found = false;
                temp = temp->prev;
            
            }

            
        }
        
        
              //   cout << "\n ID found " << iD << "\n";   
    }

    // if the third argument is a valid type --> while loop explore through all the variables in the list that are defined and update their pubpriv.
    
    
}


void Token::Print()
{
    cout << "{" << this->lexeme << " , "
         << reserved[(int) this->token_type] << " , "
         << this->line_no << "}\n";
}

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
            
            SkipComments();
        }else{
            comments = false;
            cout << "Syntax Error\n";
            exit(0);
        }
        
        
        
        
        
        
    }else{
           input.UngetChar(c);
           
           return comments;
    }
            
      
         
    

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
                
                //cout << "\n I am here too " << c << " \n";
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
                
                //cout << "\n I am here too " << c << " \n";
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
        // TODO: You can check for REALNUM, BASE08NUM and BASE16NUM here!
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
int parse_varlist(void){
    token = lexer.GetToken();
    //token.Print();
    int tempI;
    
    char* lexeme = (char*)malloc(sizeof(token.lexeme)+1);
    memcpy(lexeme, (token.lexeme).c_str(), (token.lexeme).size()+1);
    addList(lexeme);
    /*sTable* temp2 = symbolTable;
    while(temp2!=NULL){
        cout << "\n Symbol Table => Name: " << temp2->item->name << " Scope: " << temp2->item->scope << " Permission: " << temp2->item->pubpriv << "\n";
        if(temp2->next != NULL){
            temp2 = temp2->next;
        }else{
            break;
        }
        
    }*/
    
    // Check First set of ID
    if(token.token_type == ID){
        token = lexer.GetToken();
        //token.Print();
        if(token.token_type == COMMA){
            cout << "\n Rule Parsed: var_list -> ID COMMA var_list \n";
            tempI = parse_varlist();
        }else if(token.token_type == COLON){
            tempTokenType = lexer.UngetToken(token);
            cout << "\n Rule Parsed: var_list -> ID \n";
            
        
        }else{
            cout << "\n Syntax Error \n";
            
        }
    }else{
        cout << "\n Syntax Error \n";
    }    
   
    return(0);

}

// parse scope



int parse_body(void);
Token token1;
/*int parse_stmt(void){
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

int parse_unaryOperator(void){
    token = lexer.GetToken();
    
    if(token.token_type == NOT){
        cout << "\n Rule parsed: unary_operator -> NOT";
    }else{
        cout << "\n Syntax Error\n";
    }
    
    return(0);

}

int parse_binaryOperator(void){
    token = lexer.GetToken();
    //keep track of the number of bin operations in binNo
    if(token.token_type == PLUS  ){
        //return -1
        cout << "\n Rule parsed: binary_operator -> PLUS\n";
    }else if(token.token_type == MINUS ){
        //return -1
        cout << "\n Rule parsed: binary_operator -> MINUS \n";
        
    }else if(token.token_type == MULT){
        cout << "\n Rule parsed: binary_operator -> MULT\n";
        //return -1
    }else if(token.token_type == DIV ){
        //return -1
        cout << "\n Rule parsed: binary_operator -> DIV \n";
        
    }else if(token.token_type == GREATER){
        // return 2
        cout << "\n Rule parsed: binary_operator -> GREATER \n";
    }else if(token.token_type == LESS  ){
        // return 2
        cout << "\n Rule parsed: binary_operator -> LESS\n";
    }else if(token.token_type == GTEQ ){
        // return 2
        cout << "\n Rule parsed: binary_operator -> GTEQ \n";
        
    }else if(token.token_type == LTEQ){
        cout << "\n Rule parsed: binary_operator -> LTEQ\n";
        // return 2
    }else if(token.token_type == EQUAL ){
        // return 2
        cout << "\n Rule parsed: binary_operator -> EQUAL \n";
        
    }else if(token.token_type == NOTEQUAL){
        // return 2
        cout << "\n Rule parsed: binary_operator -> NOTEQUAL \n";
    }else{
        cout << "\n Syntax Error \n";
    }
    return(0);

}



int parse_primary(void){
    token = lexer.GetToken();
    
    if(token.token_type == ID  ){
        // search list for the token. If token available then return the type of the token. if not then add the token to the list
        // make its scope = "h" and make its type = -1;
        cout << "\n Rule parsed: primary -> ID\n";
    }else if(token.token_type == NUM ){
    
        cout << "\n Rule parsed: primary -> NUM \n";
        
    }else if(token.token_type == REALNUM){
        cout << "\n Rule parsed: primary -> REALNUM\n";
        
    }else if(token.token_type == TR ){
    
        cout << "\n Rule parsed: primary -> TRUE \n";
        
    }else if(token.token_type == FA){
        cout << "\n Rule parsed: primary -> FALSE \n";
    }else{
        cout << "\n Syntax Error \n";
    }
    return(0);

}

int parse_expression(void){
    int tempI;
    
    token = lexer.GetToken();
    
    if(token.token_type == ID || token.token_type == NUM || token.token_type == REALNUM || token.token_type == TR || token.token_type == FA ){
    
        cout << "\n Rule parsed: expression -> primary \n";
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_primary();
        
    
    }else if(token.token_type == PLUS || token.token_type == MINUS || token.token_type == MULT || token.token_type == DIV || token.token_type == GREATER || token.token_type == LESS || token.token_type == GTEQ || token.token_type == LTEQ || token.token_type == EQUAL || token.token_type == NOTEQUAL){
        cout << "\n Rule parsed: expression -> binary_operator expression expression \n";
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_binaryOperator();
        // in searchList and addList add an input argument that is the binNo
        // in each ID for which you do addList or searchList just update binNo.
        // if plus minus multiply divide
        tempI1 = parse_expression();

        tempI2 = parse_expression();

        //if(tempI1 != tempI2) type mismatch token.lineno C2 (this is not true if tempI1 == -1 or tempI2 == -1 )
        


        // if gt gteq ---------------
        tempI1 = parse_expression();

        tempI2 = parse_expression();
        //if(tempI1 != tempI2) type mismatch token.lineno C2 (this is not true if tempI1 == -1 or tempI2 == -1 )

        // both tempI1 and tempI2 are -1
        // is lType -1? if so then do nothing
        // if not then it implies that lType is 0 1 or 2. Then check if parser_expression returned an ID or a INT NUm or REAL.
        // if parse expression returned from INT NUM REAL do nothing
        // else if it returned from an ID, then searchList for the ID update the type of ID to lType. --> search for all IDs which have binNo same as binNo of the current ID 
        // for each such ID if its type is -1 then change their types to lType, if not type mismatch token.line_no C2

        // if only one of tempI1 or tempI2 is -1 
        // if tempI1 is -1, did tempI1 return from ID? if so then searchList for ID and make its type = tempI2 --> search for all IDs which have binNo same as binNo of the current ID 
        // for each such ID if its type is -1 then change their types to lType, if not type mismatch token.line_no C2

        // 

    }else if(token.token_type == NOT){
        cout << "\n Rule parsed: expression -> unary_operator expression \n";
        tempTokenType = lexer.UngetToken(token);
        tempI1 = parse_unaryOperator();
        tempI2 = parse_expression();

        //if parse expression returns an ID and type of that ID is -1 then make it 2 by using search list
        // if tempI2 != 2 and != -1 then Type mismatch token.line_no C3????

    }else{
        cout << "\n Syntax Error \n";
    }
    return(0);
    
}


int parse_assstmt(void){
    int tempI;
    token = lexer.GetToken();
    //cout << "\n token name " << token.lexeme << " \n";
    if(token.token_type == ID){
        // search for the token in the searchList --> the token is available, leftType = type of the available token
        // it is not available in the searchList, add the token to the list, make its type = -1; make its scope = "h".
        token = lexer.GetToken();
        //cout << "\n token name " << token.lexeme << " \n";
        if(token.token_type == EQUAL){
             token = lexer.GetToken();  
             //cout << "\n token name " << token.lexeme << " \n";
             if(token.token_type == ID || token.token_type == NUM || token.token_type == REALNUM || token.token_type == TR || token.token_type == FA || token.token_type == PLUS || token.token_type == MINUS || token.token_type == MULT || token.token_type == DIV || token.token_type == LESS || token.token_type == GREATER || token.token_type == GTEQ || token.token_type == LTEQ || token.token_type == EQUAL || token.token_type == NOTEQUAL || token.token_type == NOT){
                tempTokenType = lexer.UngetToken(token);
                tempI = parse_expression();
                //rType right type of an assigment tempI.
                //check for C1. if ltype == rtype then fine if not then TYPE MISMATCH token.line_no C1
                // if any one of lType or rType is -1 then should not throw type mismatch. 
                // if lType != -1 && rType is -1 then you search for left ID token to extract its type. searchList should return type. 
                // you have to use search list again with the right token to update the right token's type to lType 
                token = lexer.GetToken();
                //token.Print();
                if(token.token_type == SEMICOLON){
                    cout << "\n Rule parsed: assignment_stmt -> ID EQUAL expression SEMICOLON \n";
                
                }else{
                     cout << "\n HI Syntax Error " << token.token_type << " \n";    
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
}

int parse_case(void){

    int tempI;
    token = lexer.GetToken();
    if(token.token_type == CASE ){
        token = lexer.GetToken();
        if(token.token_type == NUM){
            token = lexer.GetToken();
            if(token.token_type == COLON){
                cout << "\n Rule parsed: case -> CASE NUM COLON body";
                tempI = parse_body();
            }else{
                cout << "\n Syntax Error \n";
            }
        
        }else{
            cout << "\n Syntax Error \n";
        }
    
    }else{
        cout << "\n Syntax Error \n";
    }
}

int parse_caselist(void){
    
    int tempI;
    token = lexer.GetToken();
    if(token.token_type == CASE){
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_case();
        token = lexer.GetToken();
        if(token.token_type == CASE){
            tempTokenType = lexer.UngetToken(token);
            cout << "\n Rule parsed: case_list -> case case_list \n";
            tempI = parse_caselist();
        }else if(token.token_type == RBRACE){
            tempTokenType = lexer.UngetToken(token);
            cout << "\n Rule parsed: case_list -> case  \n";
        }
    }
    return(0);
}


int parse_switchstmt(void){
    int tempI;
    
    token = lexer.GetToken();
    if(token.token_type == SWITCH){
        token = lexer.GetToken();
        if(token.token_type == LPAREN){
            tempI = parse_expression();
            // if tempI != INT then throw type error
            // else if tempI = -1 ==> parse_expresssion retunred an ID, then go and change using searchList the type of ID to 1.
            token = lexer.GetToken();
            if(token.token_type == RPAREN){
                token = lexer.GetToken();
                if(token.token_type == LBRACE){
                    tempI = parse_caselist();
                    token = lexer.GetToken();
                    if(token.token_type == RBRACE){
                        cout << "\n Rule parsed: switch_stmt -> SWITCH LPAREN expression RPAREN LBRACE case_list RBRACE \n";        
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
    }else{
        cout << "\n Syntax Error \n";
    }
    return(0);
}


int parse_whilestmt(void){
    int tempI;
    
    token = lexer.GetToken();
    if(token.token_type == WHILE){
        token = lexer.GetToken();
        if(token.token_type == LPAREN){
            tempI = parse_expression();
            // if tempI != bool then throw type error
            // else if tempI = -1 ==> parse_expresssion retunred an ID, then go and change using searchList the type of ID to 2.
            token = lexer.GetToken();
            if(token.token_type == RPAREN){
                cout << "\n Rule parsed: whilestmt -> WHILE LPAREN expression RPAREN body \n";
                tempI = parse_body();
                
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
}

int parse_ifstmt(void){
    int tempI;
    
    token = lexer.GetToken();
    if(token.token_type == IF){
        token = lexer.GetToken();
        if(token.token_type == LPAREN){
            tempI = parse_expression();

            // if tempI != bool then throw type error
            // else if tempI = -1 ==> parse_expresssion retunred an ID, then go and change using searchList the type of ID to 2.

            token = lexer.GetToken();
            if(token.token_type == RPAREN){
                cout << "\n Rule parsed: ifstmt -> IF LPAREN expression RPAREN body \n";
                tempI = parse_body();
                
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
}

int parse_stmt(void){
    int tempI;
    token = lexer.GetToken();
    if(token.token_type == ID){
        tempTokenType = lexer.UngetToken(token);
        cout << "\n Rule parsed: stmt -> assignment_stmt \n";
        tempI = parse_assstmt();        
                
    }else if(token.token_type == IF){
        tempTokenType = lexer.UngetToken(token);
        cout << "\n Rule parsed: stmt -> if_stmt";
        tempI = parse_ifstmt();
    }else if(token.token_type == WHILE){
        tempTokenType = lexer.UngetToken(token);
        cout << "\n Rule parsed: stmt -> while_stmt";
        tempI = parse_whilestmt();
    }else if(token.token_type == SWITCH){
        tempTokenType = lexer.UngetToken(token);
        cout << "\n Rule parsed: stmt -> switch_stmt";
        tempI = parse_switchstmt();
    }else{
        cout << "\n Syntax Error \n";
    }
    return(0);
}

int parse_stmtlist(void){
    
    token = lexer.GetToken();
    //token.Print();
    int tempI;
    if(token.token_type == ID || token.token_type == IF || token.token_type == WHILE || token.token_type == SWITCH){
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_stmt();
        token = lexer.GetToken();
        //token.Print();
        if(token.token_type == ID || token.token_type == IF || token.token_type == WHILE || token.token_type == SWITCH){
            tempTokenType = lexer.UngetToken(token);
            cout << "\n Rule Parsed: stmt_list -> stmt stmt_list \n";
            tempI = parse_stmtlist();
            
        }else if (token.token_type == RBRACE){
            tempTokenType = lexer.UngetToken(token);
            //printList();
            cout << "\n Rule parsed: stmt_list -> stmt \n";
        }
    }else{
        cout << "\n Syntax Error \n";
        
    }
    return(0);
}



int parse_body(void){
    
    token = lexer.GetToken();
    //token.Print();
    int tempI;
    // currentScope = (char*)malloc(sizeof(token.lexeme)+1); // different from global have any arbitrary string "h"
    //memcpy(currentScope, (token.lexeme).c_str(), (token.lexeme).size()+1);
   // addScope();
    //strcpy(currentScope, lexeme);
    
        if(token.token_type == LBRACE){
            //cout << "\n Rule Parsed: scope -> ID LBRACE public_vars private_vars stmt_list RBRACE \n";
            
            tempI = parse_stmtlist();
            token = lexer.GetToken();
            //token.Print();
            if(token.token_type == RBRACE){
                //if(symbolTable != NULL){
                //    deleteList();
                    
            
                //}
                //printScope();
                //deleteScope();
                cout << "\n Rule parsed: body -> LBRACE stmt_list RBRACE \n";
                return(0);
            }else{
                cout << "\n Syntax Error\n ";
                return(0);
            }    
                    
        
        }else if(token.token_type == END_OF_FILE){
        tempTokenType = lexer.UngetToken(token);
        return(0);
        }else{
            cout << "\n Syntax Error \n ";
            return(0);
        }

}




// parse scope end

int parse_typename(void){
    token = lexer.GetToken();
    if(token.token_type == INT || token.token_type == REAL || token.token_type == BOO){
        cout  << "\n Rule parse: type_name -> " << token.token_type << "\n"; 
        
    }else{
        cout << "\n Syntax Error \n";
    }   
    return(0);// if Int ret 0 if float ret 1 if bool ret 2
}

int parse_vardecl(void){
        int tempI;
    token = lexer.GetToken();
    if(token.token_type == ID){
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_varlist();
        token = lexer.GetToken();
        if(token.token_type == COLON){
            tempI = parse_typename();

            //use the searchList to update the types of variables that are already in the symbolTable

            token = lexer.GetToken();
            if(token.token_type == SEMICOLON){
                cout << "\n Rule parsed: var_decl -> var_list COLON type_name SEMICOLON";
            }else{
                cout << "\n Syntax Error \n";
            }
        } else{
            cout << "\n Syntax Error \n";
        } 
    }else{
        cout << "\n Syntax Error \n";
    
    }
    return(0);
}

int parse_vardecllist(void){
    int tempI;    
    token = lexer.GetToken();
    while(token.token_type == ID){
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_vardecl();
        token = lexer.GetToken();
        if(token.token_type != ID){
            cout << "\n Rule Parsed: var_decl_list -> var_decl \n";
            
        }else{
            cout << "\n Rule Parsed: var_decl_list -> var_decl var_decl_list \n";
        }
    }
    tempTokenType = lexer.UngetToken(token);
    return(0);
}


string global = "::";
// parse global vars
int parse_globalVars(void){
    token = lexer.GetToken();
    //token.Print();
    int tempI;
    
    //check first set of var_list SEMICOLON
    if(token.token_type == ID){
        tempTokenType = lexer.UngetToken(token);
        currentPrivPub = 0;
        //strcpy(currentScope,global);
        cout << "\n Rule parsed: globalVars -> var_decl_list \n";
        tempI = parse_vardecllist();
        
    
    } else{
        cout << "Syntax Error";
    }
    return(0);

}


int parse_program(void){
    
    token = lexer.GetToken();
    //token.Print();
    int tempI;
    while (token.token_type != END_OF_FILE)
    {
        // Check first set of global_vars scope
            if(token.token_type == ID){
            tempTokenType = lexer.UngetToken(token);
            //token1.Print();
            cout << "\n Rule parsed: program -> global_vars scope \n";
            tempI = parse_globalVars();
            tempI = parse_body();
                  
            }else if(token.token_type == LBRACE){
                tempTokenType = lexer.UngetToken(token);
                
                cout << "\n Rule parsed: global_vars -> epsilon \n";
                tempI = parse_body();
                
                
        
            }else if(token.token_type == END_OF_FILE){
                return(0);
            }else{
                cout << "\n Syntax Error\n";
                return(0);
            }
        token = lexer.GetToken();
        //token.Print();
    }


}
char null[] = "NULL";
int main()
{
    int i;
    //symbolTable = (sTable *) malloc(sizeof(sTable));ass
    //symbolTable = NULL;
    //scopeTable = NULL;
    //currentScope = (char *)malloc(sizeof(global)+1);
    //memcpy(currentScope,global.c_str(),global.length()+1);
    //addScope();
        i = parse_program();
      cout << "\n End of Program \n";
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
