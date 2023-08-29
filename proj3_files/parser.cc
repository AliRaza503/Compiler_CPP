#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <cstdlib>

#include "parser.h"

using namespace std;

// Functions outside the class
struct scopeResolve {
    std::string scope;
    scopeResolve *next;
};
// To store the ID lexemes that are declared.
vector<string> newDeclVars;
struct sTableEntry {
    std::string name;
    std::string scope;
    int pubpriv;
    int type;   // 1 = INT, 2 = REAL, 3 = BOOL, -1 = Undecided
    int binNo;
    bool printed = false;
};
int nextBNo = 0;

struct sTable {
    sTableEntry *item;
    sTable *prev;
    sTable *next;
};

sTable *symbolTable;
std::string currentScope;
std::string lResolve;
std::string rResolve;
scopeResolve *scopeTable;
int currentPrivPub = 0;

void addScope() {
    if (scopeTable == nullptr) {
        scopeResolve *newScopeItem = new scopeResolve;
        newScopeItem->scope = currentScope;
        newScopeItem->next = nullptr;
        scopeTable = newScopeItem;
    } else {
        scopeResolve *tempTable = scopeTable;
        while (tempTable->next != nullptr) {
            tempTable = tempTable->next;
        }
        scopeResolve *newScopeItem = new scopeResolve;
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

void addList(const std::string &lexeme, int type, int bNo = -1) {
    if (symbolTable == nullptr) {
        sTable *newEntry = new sTable;
        sTableEntry *newItem = new sTableEntry;
        newItem->name = lexeme;
        newItem->scope = currentScope;
        newItem->pubpriv = currentPrivPub;
        newItem->type = type;
        newItem->binNo = bNo;

        newEntry->item = newItem;
        newEntry->next = nullptr;
        newEntry->prev = nullptr;
        symbolTable = newEntry;
    } else {
        sTable *temp = symbolTable;
        while (temp->next != nullptr) {
            temp = temp->next;
        }
        sTable *newEntry = new sTable;
        sTableEntry *newItem = new sTableEntry;
        newItem->name = lexeme;
        newItem->scope = currentScope;
        newItem->pubpriv = currentPrivPub;
        newItem->type = type;
        newItem->binNo = bNo;

        newEntry->item = newItem;
        newEntry->next = nullptr;
        newEntry->prev = temp;
        temp->next = newEntry;
    }
}

void printScope() {
    scopeResolve *temp = scopeTable;

    cout << "\n Printing Scope Table \n";
    while (temp != nullptr && temp->next != nullptr) {
        cout << " Scope " << temp->scope << " -> ";
        temp = temp->next;
    }
    cout << " Scope " << temp->scope << " \n";
}


void printList() {
    sTable *temp = symbolTable;

    cout << "\n Printing Symbol Table \n";
    while (temp != nullptr && temp->next != nullptr) {
        cout << "\n Name: " << temp->item->name << " Scope: " << temp->item->scope << " Persmission: "
             << temp->item->pubpriv << " \n";
        temp = temp->next;
    }
    cout << "\n Name: " << temp->item->name << " Scope: " << temp->item->scope << " Persmission: "
         << temp->item->pubpriv << " \n";
}

void deleteList() {
    sTable *temp = symbolTable;

    if (temp != nullptr) {
        while (temp->next != nullptr && temp->item->scope != currentScope) {
            temp = temp->next;
        }
        if (temp->item->scope == currentScope) {
            if (currentScope != "::") {
                temp = temp->prev;
                temp->next = nullptr;
            } else {
                temp = nullptr;
            }
        }
    }
}

sTableEntry *searchList(std::string iD, int lR) {// add an argument to accept a type
    bool found = false;
    sTable *temp = symbolTable;
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
            }
        }
        while ((temp->item->name != iD) || !found) {
            if (temp->prev == nullptr && (temp->item->name != iD)) {
                if (lR == 0) {
                    lResolve = "?";
                } else {
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
    }
    // if the third argument is a valid type --> while loop explore through all the variables in the list that are defined and update their pubpriv.
    return found ? temp->item : nullptr;
}

// parse var_list
int Parser::parse_varlist() {
    token = lexer.GetToken();
    int tempI;
    string lexeme;
    lexeme = token.lexeme;
    // Check if the lexeme is already in the list
    if (searchList(lexeme, 0) != nullptr) {
        // TODO: Scope change or another var with less scope
    } else {
        // Add the lexeme to the list
        addList(lexeme, 0, nextBNo);
        newDeclVars.push_back(lexeme);
    }
    // Check First set of ID
    if (token.token_type == ID) {
        token = lexer.GetToken();
        if (token.token_type == COMMA) {
//            cout << "\n Rule Parsed: var_list -> ID COMMA var_list \n";
            tempI = parse_varlist();
        } else if (token.token_type == COLON) {
            tempTokenType = lexer.UngetToken(token);
//            cout << "\n Rule Parsed: var_list -> ID \n";
        } else {
            cout << "\n Syntax Error \n";
        }
    } else {
        cout << "\n Syntax Error \n";
    }
    return (0);
}

int Parser::parse_unaryOperator() {
    token = lexer.GetToken();

    if (token.token_type == NOT) {
//        cout << "\n Rule parsed: unary_operator -> NOT";
    } else {
        cout << "\n Syntax Error\n";
    }
    return (0);
}

int Parser::parse_binaryOperator() {
    token = lexer.GetToken();
    //keep track of the number of bin operations in binNo
    if (token.token_type == PLUS) {
//        cout << "\n Rule parsed: binary_operator -> PLUS\n";
    } else if (token.token_type == MINUS) {
//        cout << "\n Rule pared: binary_operator -> MINUS \n";
    } else if (token.token_type == MULT) {
//        cout << "\n Rule parsed: binary_operator -> MULT\n";
    } else if (token.token_type == DIV) {
//        cout << "\n Rule parsed: binary_operator -> DIV \n";
    } else if (token.token_type == GREATER) {
//        cout << "\n Rule parsed: binary_operator -> GREATER \n";
    } else if (token.token_type == LESS) {
//        cout << "\n Rule parsed: binary_operator -> LESS\n";
    } else if (token.token_type == GTEQ) {
//        cout << "\n Rule parsed: binary_operator -> GTEQ \n";
    } else if (token.token_type == LTEQ) {
//        cout << "\n Rule parsed: binary_operator -> LTEQ\n";
    } else if (token.token_type == EQUAL) {
//        cout << "\n Rule parsed: binary_operator -> EQUAL \n";
    } else if (token.token_type == NOTEQUAL) {
//        cout << "\n Rule parsed: binary_operator -> NOTEQUAL \n";
    } else {
        cout << "\n Syntax Error \n";
    }
    return token.token_type;
}

int Parser::parse_primary() {
    token = lexer.GetToken();
    if (token.token_type == ID) {
        // search list for the token. If token available then return the type of the token. if not then add the token to the list
        // make its scope = "h" and make its type = -1;
        sTableEntry *tempEntry = searchList(token.lexeme, 0);
        if (tempEntry != nullptr) {
            return static_cast<TokenType>(tempEntry->type);
        } else {
            currentScope = "h";
            addList(token.lexeme, -1);
            return -1;
        }
//        cout << "\n Rule parsed: primary -> ID\n";
    } else if (token.token_type == NUM) {
//        cout << "\n Rule parsed: primary -> NUM \n";
    } else if (token.token_type == REALNUM) {
//        cout << "\n Rule parsed: primary -> REALNUM\n";
    } else if (token.token_type == TR) {
//        cout << "\n Rule parsed: primary -> TRUE \n";
    } else if (token.token_type == FA) {
//        cout << "\n Rule parsed: primary -> FALSE \n";
    } else {
        cout << "\n Syntax Error \n";
    }
    return token.token_type;
}

int literalToType(int t) {
    if (t == NUM)
        return INT;
    else if (t == REALNUM)
        return REAL;
    else if (t == TR || t == FA)
        return BOO;
    return t;
}

void updateTypeOfAllIdsWithSameBinNo(int binNo, int lType, const Token &token, const sTableEntry *tempEntry) {
    if (binNo == -1) {
        return;
    }
    sTable *iter = symbolTable;
    while (iter != nullptr) {
        if (iter->item->binNo == tempEntry->binNo) {
            if (iter->item->type == -1 || iter->item->type == lType) {
                iter->item->type = lType;
            } else if (iter->item->binNo != -1) {
                cout << "TYPE MISMATCH " << token.line_no << " C2" << endl;
                exit(0);
            }
        }
        iter = iter->next;
    }
}

int lType = static_cast<TokenType>(-1);
int rightSideBNo = -1;
Token leftSideOfAs;

void makeAllBinNoOfaTob(int a, int b) {
    sTable *iter = symbolTable;
    while (iter != nullptr) {
        if (iter->item->binNo == a) {
            iter->item->binNo = b;
        }
        iter = iter->next;
    }
    rightSideBNo = b;
}


int Parser::parse_exp() {
    int tempI = 0;
    int t1, t2;
    token = lexer.GetToken();
    if (token.token_type == ID || token.token_type == NUM || token.token_type == REALNUM || token.token_type == TR ||
        token.token_type == FA) {
//        cout << "\n Rule parsed: expression -> primary \n";
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_primary();
        tempI = literalToType(tempI);
    } else if (token.token_type == PLUS || token.token_type == MINUS || token.token_type == MULT ||
               token.token_type == DIV || token.token_type == GREATER || token.token_type == LESS ||
               token.token_type == GTEQ || token.token_type == LTEQ || token.token_type == EQUAL ||
               token.token_type == NOTEQUAL) {
//        cout << "\n Rule parsed: expression -> binary_operator expression expression \n";
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_binaryOperator();
        // in searchList and addList add an input argument that is the binNo
        // in each ID for which you do addList or searchList just update binNo.
        t1 = parse_exp();
        // To store the token on the left side of the binary operator
        Token leftSideToken = token;
        int temp1 = literalToType(t1);
        t2 = parse_exp();
        // To store the token on the left side of the binary operator
        Token rightSideToken = token;
        int temp2 = literalToType(t2);
        if (temp1 != temp2 && t1 != -1 && t2 != -1) {
            cout << "TYPE MISMATCH " << token.line_no << " C2" << endl;
            exit(0);
        }
        // both tempI1 and tempI2 are -1
        if (t1 == -1 && t2 == -1) {
            // is lType -1? if so then do nothing
            if (lType == -1) {
                // Do Nothing
            } else {
                // if not then it implies that lType is 0 1 or 2. Then check if parser_expression returned an ID or a INT NUm or REAL.
                // if parse expression returned from INT NUM REAL do nothing
                if (t1 == INT || t1 == NUM || t2 == REAL) {
                    // Do Nothing
                } else {
                    // else if it returned from an ID, then searchList for the ID update the type of ID to lType. -->
                    sTableEntry *tempEntry = searchList(token.lexeme, 0);
                    if (tempEntry != nullptr) {
                        tempEntry->type = lType;
                        if (tempEntry->binNo == -1) {
                            if (tempEntry->type == BOO) {
                                tempEntry->binNo = ++nextBNo;
                            } else {
                                tempEntry->binNo = nextBNo;
                            }
                        }
                        // search for all IDs which have binNo same as binNo of the current ID
                        // for each such ID if its type is -1 then change their types to lType, if not type mismatch token.line_no C2
                        updateTypeOfAllIdsWithSameBinNo(tempEntry->binNo, lType, token, tempEntry);
                    }
                }
            }
        }
            // if only one of tempI1 or tempI2 is -1
            // if tempI1 is -1, did tempI1 return from ID? if so then searchList for ID and make its type = tempI2 --> search for all IDs which have binNo same as binNo of the current ID
            // for each such ID if its type is -1 then change their types to lType, if not type mismatch token.line_no C2
        else if (t1 == -1 || t2 == -1) {    // Will act as XOR because of the previous if statement.
            if (t1 == -1) {
                sTableEntry *tempEntry = searchList(leftSideToken.lexeme, 0);
                if (tempEntry != nullptr) {
                    tempEntry->type = temp2;
                    if (tempEntry->binNo == -1) {
                        if (tempEntry->type == BOO) {
                            tempEntry->binNo = ++nextBNo;
                        } else {
                            tempEntry->binNo = nextBNo;
                        }
                    }
                    temp1 = temp2;
                    updateTypeOfAllIdsWithSameBinNo(tempEntry->binNo, temp2, leftSideToken, tempEntry);
                }
            } else {
                sTableEntry *tempEntry = searchList(token.lexeme, 0);
                if (tempEntry != nullptr) {
                    tempEntry->type = temp1;
                    if (tempEntry->binNo == -1)
                        tempEntry->binNo = nextBNo;
                    temp2 = temp1;
                    updateTypeOfAllIdsWithSameBinNo(tempEntry->binNo, temp1, token, tempEntry);
                }
            }
        }

        // Check if the tokens on both sides of the binary operator are identifiers. If both are identifiers then assign a unique biNo to each of them.
        if (leftSideToken.token_type == ID && rightSideToken.token_type == ID) {
            sTableEntry *leftTokenEntry = searchList(leftSideToken.lexeme, 0);
            sTableEntry *rightTokenEntry = searchList(rightSideToken.lexeme, 0);
            if (leftTokenEntry != nullptr && rightTokenEntry != nullptr) {
                int leftBinNo = leftTokenEntry->binNo;
                int rightBinNo = rightTokenEntry->binNo;
                // If both are -1 then assign a new binNo to both of them
                if (leftBinNo == -1 && rightBinNo == -1) {
                    leftTokenEntry->binNo = nextBNo;
                    rightTokenEntry->binNo = nextBNo;
                    rightSideBNo = nextBNo;
                } else if (leftBinNo == -1 && rightBinNo != -1) {
                    leftTokenEntry->binNo = rightBinNo;
                    rightSideBNo = rightBinNo;
                } else if (leftBinNo != -1 && rightBinNo == -1) {
                    rightTokenEntry->binNo = leftBinNo;
                    rightSideBNo = leftBinNo;
                } else if (leftBinNo != -1 && rightBinNo != -1) {
                    if (leftBinNo == rightBinNo) {
                        rightSideBNo = leftBinNo;
                        // Do Nothing
                    } else if (t1 != 3 && t2 != 3) {
                        // make all binNo of leftBinNo to rightBinNo in the symbol table
                        makeAllBinNoOfaTob(leftBinNo, rightBinNo);
                    }
                }
            }
        }


        // if plus minus multiply divide
        if (tempI == PLUS || tempI == MINUS || tempI == MULT || tempI == DIV) {
            return temp1;      //t1 and t2 are of the same type return t1
        } else if (tempI == GREATER || tempI == LESS || tempI == GTEQ || tempI == LTEQ || tempI == EQUAL ||
                   tempI == NOTEQUAL) {
            return BOO;     // It was a boolean operator
        }

    } else if (token.token_type == NOT) {
//        cout << "\n Rule parsed: expression -> unary_operator expression \n";
        tempTokenType = lexer.UngetToken(token);
        t1 = parse_unaryOperator();
        t2 = parse_exp();
        t2 = literalToType(t2);
        //if parse expression returns an ID and type of that ID is -1 then make it 3 by using search list
        if (t2 == -1) {     //If it is not returned and ID then t2 will be 0
            sTableEntry *tempEntry = searchList(token.lexeme, 0);
            if (tempEntry != nullptr) {
                tempEntry->type = 3;
            }
        }
            // if parse expression returns an ID and type of that ID is not -1 then type mismatch token.line_no C3
        else if (t2 != BOO) {
            cout << "TYPE MISMATCH " << token.line_no << " C3" << endl;
            exit(0);
        }
        return t2;
    } else {
        cout << "\n Syntax Error \n";
    }
    return tempI;
}

int Parser::parse_expression() {
    nextBNo++;
    return parse_exp();
}

int Parser::parse_assstmt() {
    int tempI;
    string addedTokenLex;
    lType = -1;
    token = lexer.GetToken();
    if (token.token_type == ID) {
        leftSideOfAs = token;
        // search for the token in the searchList --> the token is available, leftType = type of the available token
        sTableEntry *tempEntry = searchList(token.lexeme, 0);
        if (tempEntry != nullptr) {
            lType = tempEntry->type;
        }
            // it is not available in the searchList, add the token to the list, make its type = -1; make its scope = "h".
        else {
            currentScope = "h";
            addList(token.lexeme, -1);
            addedTokenLex = token.lexeme;
        }
        token = lexer.GetToken();
        if (token.token_type == EQUAL) {
            token = lexer.GetToken();
            if (token.token_type == ID || token.token_type == NUM || token.token_type == REALNUM ||
                token.token_type == TR || token.token_type == FA || token.token_type == PLUS ||
                token.token_type == MINUS || token.token_type == MULT || token.token_type == DIV ||
                token.token_type == LESS || token.token_type == GREATER || token.token_type == GTEQ ||
                token.token_type == LTEQ || token.token_type == EQUAL || token.token_type == NOTEQUAL ||
                token.token_type == NOT) {
                tempTokenType = lexer.UngetToken(token);
                Token rightSideToken = Token();
                rightSideToken.token_type = END_OF_FILE;
                if (token.token_type == ID) {
                    rightSideToken = token;
                }
                rightSideBNo = -1;
                tempI = parse_expression();
                tempI = literalToType(tempI);
                //rType right type of an assigment tempI.
                int rType = tempI;
                //check for C1. if ltype == rtype then fine if not then TYPE MISMATCH token.line_no C1
                if (lType != rType && lType != -1 && rType != -1) {
                    cout << "TYPE MISMATCH " << token.line_no << " C1\n";
                    exit(0);
                }
                sTableEntry *entry = searchList(leftSideOfAs.lexeme, 0);
                // if any one of lType or rType is -1 then should not throw type mismatch.
                // if lType != -1 && rType is -1 then you search for left ID token to extract its type. searchList should return type.
                // A case is when on the left side there is a variable being reused and on the right side there is a new variable being declared.
                //    a = 1;
                //    a = b;
                if (lType != -1 && rType == -1) {
                    if (entry != nullptr && entry->type != -1) {
                        rType = entry->type;
                        if (entry->binNo == -1) {
                            entry->binNo = rightSideBNo;    // left side elems have binNo = rightSideBNo
                        } else {
                            makeAllBinNoOfaTob(entry->binNo, rightSideBNo);
                        }
                        updateTypeOfAllIdsWithSameBinNo(rightSideBNo, rType, token, entry);

                    }
                }
                // Updating type of the added token to the list
                // A case is when on the left side there is a new variable being declared and on the right side there is a variable being reused.
                if (lType == -1 && rType != -1) {
                    // TODO: if right side is of type Bool then it is not necessary that the items on the right side are all bool but for numeric type this is true.
                    if (entry != nullptr) {
                        if (rightSideBNo != -1 && rType != BOO)     // right side elems have binNo = nextBNo
                        {
                            if (entry->binNo != -1) {
                                makeAllBinNoOfaTob(entry->binNo, rightSideBNo);
                            } else {
                                entry->binNo = rightSideBNo;
                            }
                        } else if (entry->binNo == -1 && rType != BOO) {
                            makeAllBinNoOfaTob(-1, nextBNo);
                        } else if (entry->binNo == -1) {  //rType == BOO
                            entry->binNo = ++nextBNo;
                        }
                        if (entry->type != -1 && entry->type != rType) {
                            cout << "TYPE MISMATCH " << token.line_no << " C1\n";
                            exit(0);
                        }
                        entry->type = rType;
                    }
                } else if (lType == -1 && rType == -1) {
                    if (entry != nullptr) {
                        if (rightSideBNo != -1)     // right side elems have binNo = nextBNo
                        {
                            if (entry->binNo != -1) {
                                makeAllBinNoOfaTob(entry->binNo, rightSideBNo);
                            } else {
                                entry->binNo = rightSideBNo;
                            }
                        } else if (entry->binNo == -1) {
                            makeAllBinNoOfaTob(-1, nextBNo);
                        }
                        if (entry->type != -1 && entry->type != rType) {
                            cout << "TYPE MISMATCH " << token.line_no << " C1\n";
                            exit(0);
                        }
                        entry->type = rType;
                    }
                }
                // you have to use search list again with the right token to update the right token's type to lType
                // if right side is an ID, then its BinNo is to be considered.
                if (entry->type != -1) {
                    updateTypeOfAllIdsWithSameBinNo(entry->binNo, entry->type, token, entry);
                }

                token = lexer.GetToken();
                if (token.token_type == SEMICOLON) {
//                    cout << "\n Rule parsed: assignment_stmt -> ID EQUAL expression SEMICOLON \n";
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

int Parser::parse_case() {
    int tempI;
    token = lexer.GetToken();

    if (token.token_type == CASE) {
        token = lexer.GetToken();
        if (token.token_type == NUM) {
            token = lexer.GetToken();
            if (token.token_type == COLON) {
//                cout << "\n Rule parsed: case -> CASE NUM COLON body";
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

int Parser::parse_caselist() {

    int tempI;
    token = lexer.GetToken();
    if (token.token_type == CASE) {
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_case();
        token = lexer.GetToken();
        if (token.token_type == CASE) {
            tempTokenType = lexer.UngetToken(token);
//            cout << "\n Rule parsed: case_list -> case case_list \n";
            tempI = parse_caselist();
        } else if (token.token_type == RBRACE) {
            tempTokenType = lexer.UngetToken(token);
//            cout << "\n Rule parsed: case_list -> case  \n";
        }
    }
    return (0);
}


int Parser::parse_switchstmt() {
    int tempI;
    token = lexer.GetToken();
    if (token.token_type == SWITCH) {
        token = lexer.GetToken();
        if (token.token_type == LPAREN) {
            tempI = parse_expression();
            // if tempI != INT then throw type error
            if (tempI != INT && tempI != -1) {
                cout << "TYPE MISMATCH " << token.line_no << " C5" << endl;
                exit(0);
            }
            if (tempI == -1) {
                // search for the ID in the list and get its type.
                sTableEntry *entry = searchList(token.lexeme, 0);
                if (entry->binNo == -1) {
                    entry->binNo = nextBNo;
                }
                if (entry != nullptr) {
                    updateTypeOfAllIdsWithSameBinNo(entry->binNo, INT, token, entry);
                }
            }
            token = lexer.GetToken();
            if (token.token_type == RPAREN) {
                token = lexer.GetToken();
                if (token.token_type == LBRACE) {
                    tempI = parse_caselist();
                    token = lexer.GetToken();
                    if (token.token_type == RBRACE) {
//                        cout << "\n Rule parsed: switch_stmt -> SWITCH LPAREN expression RPAREN LBRACE case_list RBRACE \n";
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


int Parser::parse_whilestmt() {
    int tempI;
    token = lexer.GetToken();

    if (token.token_type == WHILE) {
        token = lexer.GetToken();
        if (token.token_type == LPAREN) {
            tempI = parse_expression();
            // if tempI != bool then throw type error
            if (tempI != BOO && tempI != -1) {
                cout << "TYPE MISMATCH " << token.line_no << " C4" << endl;
                exit(0);
            }
                // else if tempI = -1 ==> parse_expression retunred an ID, then go and change using searchList the type of ID to 3.
            else if (tempI == -1) {
                // search for the ID in the list and get its type.
                sTableEntry *entry = searchList(token.lexeme, 0);
                if (entry->binNo == -1) {
                    entry->binNo = nextBNo;
                }
                if (entry != nullptr) {
                    updateTypeOfAllIdsWithSameBinNo(entry->binNo, BOO, token, entry);
                }
            }
            token = lexer.GetToken();
            if (token.token_type == RPAREN) {
//                cout << "\n Rule parsed: whilestmt -> WHILE LPAREN expression RPAREN body \n";
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

int Parser::parse_ifstmt() {
    int tempI;
    token = lexer.GetToken();
    if (token.token_type == IF) {
        token = lexer.GetToken();
        if (token.token_type == LPAREN) {
            tempI = parse_expression();
            // if tempI != bool then throw type error
            // else if tempI = -1 ==> parse_expression retunred an ID, then go and change using searchList the type of ID to 2.
            if (tempI != BOO && tempI != -1) {
                cout << "TYPE MISMATCH " << token.line_no << " C4" << endl;
                exit(0);
            } else if (tempI == -1) {
                // search for the ID in the list and get its type.
                sTableEntry *entry = searchList(token.lexeme, 0);
                if (entry->binNo == -1) {
                    entry->binNo = nextBNo;
                }
                if (entry != nullptr) {
                    updateTypeOfAllIdsWithSameBinNo(entry->binNo, BOO, token, entry);
                }
            }
            token = lexer.GetToken();
            if (token.token_type == RPAREN) {
//                cout << "\n Rule parsed: ifstmt -> IF LPAREN expression RPAREN body \n";
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

int Parser::parse_stmt() {
    int tempI;
    token = lexer.GetToken();
    if (token.token_type == ID) {
        tempTokenType = lexer.UngetToken(token);
//        cout << "\n Rule parsed: stmt -> assignment_stmt \n";
        tempI = parse_assstmt();
    } else if (token.token_type == IF) {
        tempTokenType = lexer.UngetToken(token);
//        cout << "\n Rule parsed: stmt -> if_stmt";
        tempI = parse_ifstmt();
    } else if (token.token_type == WHILE) {
        tempTokenType = lexer.UngetToken(token);
//        cout << "\n Rule parsed: stmt -> while_stmt";
        tempI = parse_whilestmt();
    } else if (token.token_type == SWITCH) {
        tempTokenType = lexer.UngetToken(token);
//        cout << "\n Rule parsed: stmt -> switch_stmt";
        tempI = parse_switchstmt();
    } else {
        cout << "\n Syntax Error \n";
    }
    return (0);
}

int Parser::parse_stmtlist() {
    token = lexer.GetToken();
    int tempI;
    if (token.token_type == ID || token.token_type == IF || token.token_type == WHILE || token.token_type == SWITCH) {
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_stmt();
        token = lexer.GetToken();
        if (token.token_type == ID || token.token_type == IF || token.token_type == WHILE ||
            token.token_type == SWITCH) {
            tempTokenType = lexer.UngetToken(token);
//            cout << "\n Rule Parsed: stmt_list -> stmt stmt_list \n";
            tempI = parse_stmtlist();
        } else if (token.token_type == RBRACE) {
            tempTokenType = lexer.UngetToken(token);
//            cout << "\n Rule parsed: stmt_list -> stmt \n";
        }
    } else {
        cout << "\n Syntax Error \n";
    }
    return (0);
}


int Parser::parse_body() {
    token = lexer.GetToken();
    int tempI;

    if (token.token_type == LBRACE) {
        tempI = parse_stmtlist();
        token = lexer.GetToken();
        if (token.token_type == RBRACE) {
//            cout << "\n Rule parsed: body -> LBRACE stmt_list RBRACE \n";
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
int Parser::parse_typename() {
    token = lexer.GetToken();
    if (token.token_type == INT || token.token_type == REAL || token.token_type == BOO) {
//        cout << "\n Rule parse: type_name -> " << token.token_type << "\n";

    } else {
        cout << "\n Syntax Error \n";
        exit(0);
    }
    // if Int ret 1 if float ret 2 if bool ret 3
    if (token.token_type == INT) {
        return (1);
    } else if (token.token_type == REAL) {
        return (2);
    } else {
        return (3);
    }
}

int Parser::parse_vardecl() {
    int tempI;
    token = lexer.GetToken();
    if (token.token_type == ID) {
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_varlist();
        token = lexer.GetToken();
        if (token.token_type == COLON) {
            tempI = parse_typename();
            //call the searchList() to update the types of variables that are already in the symbolTable
            for (int i = 0; i < newDeclVars.size(); i++) {
                sTableEntry *t = searchList(newDeclVars[i], 0);
                // t cannot be null
                t->type = tempI;
            }
            newDeclVars.clear();
            token = lexer.GetToken();
            if (token.token_type == SEMICOLON) {
//                cout << "\n Rule parsed: var_decl -> var_list COLON type_name SEMICOLON";
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

int Parser::parse_vardecllist() {
    int tempI;
    token = lexer.GetToken();
    while (token.token_type == ID) {
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_vardecl();
        token = lexer.GetToken();
        if (token.token_type != ID) {
//            cout << "\n Rule Parsed: var_decl_list -> var_decl \n";
        } else {
//            cout << "\n Rule Parsed: var_decl_list -> var_decl var_decl_list \n";
        }
    }
    tempTokenType = lexer.UngetToken(token);
    return (0);
}


string global = "::";

// parse global vars
int Parser::parse_globalVars() {
    token = lexer.GetToken();
    int tempI;
    //check first set of var_list SEMICOLON
    if (token.token_type == ID) {
        tempTokenType = lexer.UngetToken(token);
        currentPrivPub = 0;
//        cout << "\n Rule parsed: globalVars -> var_decl_list \n";
        tempI = parse_vardecllist();
    } else {
        cout << "Syntax Error";
    }
    return (0);
}

void printVars() {
    sTable *temp = symbolTable;
    while (temp != nullptr) {
        if (!temp->item->printed) {
            if (temp->item->type == 1) {
                cout << temp->item->name << ": " << "int #\n";
                temp->item->printed = true;
            } else if (temp->item->type == 2) {
                cout << temp->item->name << ": " << "real #\n";
                temp->item->printed = true;
            } else if (temp->item->type == 3) {
                cout << temp->item->name << ": " << "bool #\n";
                temp->item->printed = true;
            } else {
                temp->item->printed = false;
                bool firstInLine = true;
                // Print all with the same binNo in the same line
                sTable *iter = temp;
                while (iter != nullptr) {
                    if (iter->item->binNo == temp->item->binNo && iter->item->type == -1) {
                        if (!firstInLine) {
                            cout << ", ";
                        }
                        iter->item->printed = true;
                        firstInLine = false;
                        cout << iter->item->name;
                    }
                    iter = iter->next;
                }
                cout << ": " << "? #\n";
            }
        }
        temp = temp->next;
    }
}

int Parser::parse_program() {
    token = lexer.GetToken();
    int tempI;
    while (token.token_type != END_OF_FILE) {
        // Check first set of global_vars scope
        if (token.token_type == ID) {
            tempTokenType = lexer.UngetToken(token);
//            cout << "\n Rule parsed: program -> global_vars scope \n";
            tempI = parse_globalVars();
            tempI = parse_body();
        } else if (token.token_type == LBRACE) {
            tempTokenType = lexer.UngetToken(token);
//            cout << "\n Rule parsed: global_vars -> epsilon \n";
            tempI = parse_body();
        } else if (token.token_type == END_OF_FILE) {
            return (0);
        } else {
            cout << "\n Syntax Error\n";
            return (0);
        }
        printVars();
        token = lexer.GetToken();
    }
}

char null[] = "nullptr";

int main() {
    int i;
    Parser *parseProgram = new Parser();
    i = parseProgram->parse_program();
//    cout << "\n End of Program \n";
    // Freeing the symbol table
    sTable *temp = symbolTable;
    while (temp != nullptr) {
        sTable *temp2 = temp;
        temp = temp->next;
        delete temp2;
    }
    delete temp;
}
