#include "error_check.h"



extern int err; //from parser
extern struct Node* root; //from parser

int identifierIsPointer(SymbolTable* symtab, char* ident){
	ListEntry* tmp = symtab->list;

	//search for variable in current table
	while(tmp){
		if(!strcmp(tmp->identifier, ident)){
			return tmp->isPointer;
		}
		tmp = tmp->next;
	}

	tmp = symtab->parent->list;

	//search for variable in parent table
	while(tmp){
		if(!strcmp(tmp->identifier, ident)){
			return tmp->isPointer;
		}
		tmp = tmp->next;
	}

	//variable not found, shouldn't happen
	return 0;
}

int checkForPointerInOp(Node* node, SymbolTable* symtab){
	Node* first_op = FIRSTCHILD(node);
	Node* second_op = SECONDCHILD(node);

	//check first operand
	if(first_op->kind == ValueOf){
		first_op = FIRSTCHILD(first_op);
		if(identifierIsPointer(symtab, first_op->u.identifier)){
			fprintf(stderr, "line %d: \033[1;31merror:\033[0m cannot use pointer in operation\n", node->lineno);
			err = 1;
			return 1;
		}
	}

	//check second operand
	if(second_op->kind == ValueOf){
		second_op = FIRSTCHILD(second_op);
		if(identifierIsPointer(symtab, second_op->u.identifier)){
			fprintf(stderr, "line %d: \033[1;31merror:\033[0m cannot use pointer in operation\n", node->lineno);
			err = 1;
			return 1;
		}
	}

	return 0;
}

int checkForPointerInPrint(Node* node, SymbolTable* symtab){
	Node* first_op = FIRSTCHILD(node);

	if(first_op->kind == ValueOf){
		first_op = FIRSTCHILD(first_op);
		if(identifierIsPointer(symtab, first_op->u.identifier)){
			fprintf(stderr, "line %d: \033[1;31merror:\033[0m cannot use pointer in \033[1mprint\033[0m call\n", node->lineno);
			err = 1;
			return 1;
		}
	}
}

int getLinenoVar(SymbolTable *symtab, char *ident){
	ListEntry* tmp = symtab->list;

	//search for variable in current table
	while(tmp){
		if(!strcmp(tmp->identifier, ident)){
			return tmp->lineno;
		}
		tmp = tmp->next;
	}

	tmp = symtab->parent->list;

	//search for variable in parent table
	while(tmp){
		if(!strcmp(tmp->identifier, ident)){
			return tmp->lineno;
		}
		tmp = tmp->next;
	}

	//variable has no lineno, shouldn't happen
	return - 1;
}

int getTypeVar(SymbolTable *symtab, char *ident){
	ListEntry* tmp = symtab->list;

	//search for variable in current table
	while(tmp){
		if(!strcmp(tmp->identifier, ident)){
			return tmp->type;
		}
		tmp = tmp->next;
	}

	tmp = symtab->parent->list;

	//search for variable in parent table
	while(tmp){
		if(!strcmp(tmp->identifier, ident)){
			return tmp->type;
		}
		tmp = tmp->next;
	}

	//not a variable (boolean or arithmetic operation)
	return - 1;
}



void error_check(Node *node, SymbolTable *symtab){
	int type, pointer;
	Node* node_tmp, *rec, *rec_tmp;
	char id_func[512], id_arg[512];
	int types[512];
	int ispointer[512];
	Node* args;
	Node* arg;
	int i, nb_param;
	int left_type, right_type;
	char id[512];
	int isPointer_left, isPointer_right;

	if(!node)
		return;

	switch(node->kind){
		case Program :
			error_check(SECONDCHILD(node) , symtab); 
			break;

		case FuncDecList:
			node_tmp = FIRSTCHILD(node);
			error_check(node_tmp , symtab);  //error_check the first function
			while(node_tmp->nextSibling){ //error_check all the other functions
				error_check(node_tmp->nextSibling , symtab);
				node_tmp = node_tmp->nextSibling;
			}
			break;
		case FuncDec:
			//checking for return type
			if(FIRSTCHILD(node)->kind == Pointer){

				//go to Body node
				node_tmp = FOURTHCHILD(node);

				//go to first statement of StmtList node
				node_tmp = FIRSTCHILD(SECONDCHILD(node_tmp));

				while(node_tmp){
					if(node_tmp->kind == Return){
						if(!(identifierIsPointer(node->st, FIRSTCHILD(FIRSTCHILD(node_tmp))->u.identifier))){
							fprintf(stderr, "line %d:", node_tmp->lineno);
							fprintf(stderr, "\033[1;35m warning:\033[0m return makes integer from pointer without a cast\n" );
						}
					}

					node_tmp = node_tmp->nextSibling;
				}

			}

			error_check(FOURTHCHILD(node) , node->st); //corps
			break;

		case Body:
			error_check(FIRSTCHILD(node) , symtab);
			error_check(SECONDCHILD(node) , symtab);
			break;

		case VarDeclList:			
			error_check(FIRSTCHILD(node) , symtab);
			break;

		case Int:
			error_check(FIRSTCHILD(node) , symtab); //vardec
			error_check(node->nextSibling , symtab); //if char variables are declared
			break;

		case Char:
			error_check(FIRSTCHILD(node) , symtab); //vardec
			error_check(node->nextSibling , symtab); //if int variables are declared
			break;

		case VarDec:
			error_check(node->nextSibling , symtab);
			break;

		case StmtList:
			node_tmp = FIRSTCHILD(node);
			error_check(node_tmp , symtab);  //error_check the first statement
			while(node_tmp->nextSibling){ //error_check all the other statements
				error_check(node_tmp->nextSibling , symtab);
				node_tmp = node_tmp->nextSibling;
			}
			break;

		case Assign:		
			error_check(FIRSTCHILD(node), symtab);
			
			if (FIRSTCHILD(node)->kind == Identifier){
				strcpy(id, FIRSTCHILD(node)->u.identifier);
				left_type = getTypeVar(symtab, id);
				isPointer_left = identifierIsPointer(symtab, id);
			}
			
			else { //pointer
				strcpy(id, FIRSTCHILD(FIRSTCHILD(node))->u.identifier);
				left_type = getTypeVar(symtab, id);
				isPointer_left = 1;
				if (identifierIsPointer(symtab, id)){
					isPointer_left = 0;
				}
			}

			node_tmp = SECONDCHILD(node);

			switch (node_tmp->kind){
			
				case CharLiteral :
					isPointer_right = 0;
					right_type = CHAR;
					break;
					
				case ValueOf :
					strcpy(id, FIRSTCHILD(node_tmp)->u.identifier);
					right_type = getTypeVar(symtab, id);	
					isPointer_right = identifierIsPointer(symtab, id);
					break;
					
				case Pointer :
					strcpy(id, FIRSTCHILD(FIRSTCHILD(node_tmp))->u.identifier);				
					right_type = getTypeVar(symtab, id);
					isPointer_right = 1;
					if (identifierIsPointer(symtab, id)){
						isPointer_right = 0;
					}
					break;
					
				case Deref :
					strcpy(id, FIRSTCHILD(FIRSTCHILD(node_tmp))->u.identifier);								
					right_type = getTypeVar(symtab, id);
					isPointer_right = 1;
					if (identifierIsPointer(symtab, id)){
						isPointer_right = !isPointer_left;
					}	
					break;
					
				case FuncCall :
					right_type = getTypeVar(symtab->parent, FIRSTCHILD(node_tmp)->u.identifier);
					isPointer_right = identifierIsPointer(symtab->parent, symtab->name);
					if (right_type == VOID_TYPE){
                    	fprintf(stderr, "line %d: \033[1;31merror:\033[0m assigning return value from a function that is of return type \033[1m'void'\033[0m\n", node->lineno);
                   		err = 1;
					}
					break;
				
				case IntLiteral:
					isPointer_right = 0;
					right_type = INT;
					break;
				//operations, such as Add, Sub, Or, Eq...	
				default :
					rec = SECONDCHILD(node);
					rec_tmp = FIRSTCHILD(rec);
					//recursively look through all the operand of the right value operation 
					while(rec_tmp->kind != CharLiteral && rec_tmp->kind != IntLiteral && 
						rec_tmp->kind != FuncCall && rec_tmp->kind != ValueOf && rec_tmp->kind != Pointer){

						if(rec_tmp->nextSibling->kind == FuncCall){
							//get function name from call
							right_type = getTypeVar(symtab->parent, FIRSTCHILD(rec_tmp->nextSibling)->u.identifier);
							if (right_type == VOID_TYPE){
                    			fprintf(stderr, "line %d: \033[1;31merror:\033[0m assigning return value from a function that is of return type \033[1m'void'\033[0m\n", node->lineno);
                   				err = 1;
							}					
						}

						rec = FIRSTCHILD(rec);
						rec_tmp = FIRSTCHILD(rec);
					}

					//finish with the last two values
					if(FIRSTCHILD(rec)->kind == FuncCall){
						//get function name from call
						right_type = getTypeVar(symtab->parent, FIRSTCHILD(FIRSTCHILD(rec))->u.identifier);
						if (right_type == VOID_TYPE){
                    		fprintf(stderr, "line %d: \033[1;31merror:\033[0m assigning return value from a function that is of return type \033[1m'void'\033[0m\n", node->lineno);
                   			err = 1;
						}	
					}

					//neg node has no secondchild
					if(rec->kind == Neg)
						break;

					if(SECONDCHILD(rec)->kind == FuncCall){
						//get function name from call
						right_type = getTypeVar(symtab->parent, FIRSTCHILD(SECONDCHILD(rec))->u.identifier);
						if (right_type == VOID_TYPE){
                    		fprintf(stderr, "line %d: \033[1;31merror:\033[0m assigning return value from a function that is of return type \033[1m'void'\033[0m\n", node->lineno);
                   			err = 1;
						}	
					}

					right_type = INT;	
					break;	
			
			}	
			
			if (isPointer_right != isPointer_left){
				fprintf(stderr, "line %d:", node->lineno);
				fprintf(stderr, "\033[1;35m warning:\033[0m assignment makes pointer from integer\n");
			}
			
			//if the left type is char and is assigned an int, display a warning
			if (left_type == CHAR && right_type == INT){
				fprintf(stderr, "line %d:", node->lineno);
				fprintf(stderr, "\033[1;35m warning:\033[0m assigning \033[1m'int'\033[0m type to \033[1m'char'\033[0m left-value \033[1;36m%s\033[0m\n", id);
			}

			error_check(SECONDCHILD(node) , symtab);
			break;

		case Add:
			checkForPointerInOp(node, symtab);
			error_check(FIRSTCHILD(node), symtab); //left value of add
			error_check(SECONDCHILD(node), symtab); //right value of add
			break;

		case Sub:
			checkForPointerInOp(node, symtab);
			error_check(FIRSTCHILD(node), symtab); //left value of sub
			error_check(SECONDCHILD(node), symtab); //right value of sub
			break;

		case Mult:
			checkForPointerInOp(node, symtab);
			error_check(FIRSTCHILD(node), symtab); //left value of mult
			error_check(SECONDCHILD(node), symtab); //right value of mult
			break;

		case Div:
			checkForPointerInOp(node, symtab);
			error_check(FIRSTCHILD(node), symtab); //left value of div
			error_check(SECONDCHILD(node), symtab); //right value of div
			break;

		case Modulo:
			checkForPointerInOp(node, symtab);
			error_check(FIRSTCHILD(node), symtab); //left value of modulo
			error_check(SECONDCHILD(node), symtab); //right value of modulo
			break;

		case Print:
			checkForPointerInPrint(node, symtab);
			error_check(FIRSTCHILD(node), symtab); //identifier that is printed
			break;

		case Readc:
			type = getTypeVar(symtab, FIRSTCHILD(FIRSTCHILD(node))->u.identifier);
			if(type == 255){
				fprintf(stderr, "line %d:", node->lineno);
				fprintf(stderr, "\033[1;35m warning:\033[0m call to readc on int, type \033[1m'char'\033[0m expected\n" );
			}

			error_check(FIRSTCHILD(node), symtab);
			break;

		case Reade:
			type = getTypeVar(symtab, FIRSTCHILD(FIRSTCHILD(node))->u.identifier);
			if(type == 256){
				fprintf(stderr, "line %d:", node->lineno);
				fprintf(stderr, "\033[1;35m warning:\033[0m call to reade on \033[1m'char'\033[0m, type \033[1m'int'\033[0m expected\n" );
			}

			error_check(FIRSTCHILD(node), symtab);
			break;

		case FuncCall:
			strcpy(id_func, FIRSTCHILD(node)->u.identifier);
			
			//fills the tabs types and isPointer
			nb_param = lookup_types(id_func, types, ispointer);
			args = SECONDCHILD(node);
			arg = FIRSTCHILD(args);
			
			for (i = 0 ; i < nb_param ; i++){
				if (arg == NULL){
					fprintf(stderr, "line %d:", node->lineno);
					fprintf(stderr, " \033[1;31merror:\033[0m too few arguments to function ‘%s’\n", id_func );		
					break;			
				}
			
				switch (arg->kind){
				
					case IntLiteral:
						type = INT;
						pointer = 0;
						break;
				
					case CharLiteral:
						type = CHAR;
						pointer = 0;
						break;
				
					case ValueOf:
						strcpy(id_arg, FIRSTCHILD(arg)->u.identifier);
						type = getTypeVar(symtab, id_arg);
						pointer = identifierIsPointer(symtab, id_arg);						
						break;
					
					case Pointer:
						strcpy(id_arg, FIRSTCHILD(FIRSTCHILD(arg))->u.identifier);
						type = getTypeVar(symtab, id_arg);
						pointer = 1;
						if (identifierIsPointer(symtab, id_arg))
							pointer = 0;
						error_check(arg, symtab);						
						break;
					
					case Deref:
						strcpy(id_arg, FIRSTCHILD(FIRSTCHILD(arg))->u.identifier);
						type = getTypeVar(symtab, id_arg);	
						pointer = 1;
						if (identifierIsPointer(symtab, id_arg)){
							pointer = !ispointer[i];
						}											
						break;		
				}
				
				if (pointer != ispointer[i]) {
					fprintf(stderr, "line %d:", node->lineno);
					fprintf(stderr, "\033[1;35m warning:\033[0m passing argument %d of ‘%s’ makes pointer from integer without a cast \n", i+1, id_func);					
				}				
				
				else if (type == INT && types[i] == CHAR){
					fprintf(stderr, "line %d:", node->lineno);
					fprintf(stderr, "\033[1;35m warning:\033[0m passing argument %d \033[1m'int'\033[0m type when expected a  \033[1m'char'\033[0m type argument \033[1;36m\033[0m\n", i+1);					
				}
				
				arg = arg->nextSibling;
			}
			break;

		case ValueOf:
			error_check(FIRSTCHILD(node), symtab); //identifier		
			break;
			
		case Or:
			error_check(FIRSTCHILD(node), symtab);
			error_check(SECONDCHILD(node), symtab);		
			break;			
			
		case And:
			error_check(FIRSTCHILD(node), symtab);
			error_check(SECONDCHILD(node), symtab);
			break;

		case IfThen:
			//first child is the condition
			error_check(FIRSTCHILD(node), symtab);
			error_check(SECONDCHILD(node), symtab);
					
			break;

		case IfThenElse:
			error_check(FIRSTCHILD(node), symtab);
			error_check(SECONDCHILD(node), symtab);
			error_check(THIRDCHILD(node), symtab);

			break;


		case While:
			error_check(SECONDCHILD(node), symtab);
			error_check(FIRSTCHILD(node), symtab);
				
			break;
			
		case Inf:
			checkForPointerInOp(node, symtab);
			error_check(FIRSTCHILD(node), symtab);
			error_check(SECONDCHILD(node), symtab);
			
		case Sup:
			checkForPointerInOp(node, symtab);
			error_check(FIRSTCHILD(node), symtab);
			error_check(SECONDCHILD(node), symtab);
			break;	
			
		case InfEq:
			checkForPointerInOp(node, symtab);
			error_check(FIRSTCHILD(node), symtab);
			error_check(SECONDCHILD(node), symtab);
			break;	
			
		case SupEq:
			checkForPointerInOp(node, symtab);
			error_check(FIRSTCHILD(node), symtab);
			error_check(SECONDCHILD(node), symtab);
			break;					

		case Eq:
			error_check(FIRSTCHILD(node), symtab);
			error_check(SECONDCHILD(node), symtab);
			break;
			
		case Diff:
			error_check(FIRSTCHILD(node), symtab);
			error_check(SECONDCHILD(node), symtab);
			break;	

		case Return:
            type = getTypeVar(symtab->parent, symtab->name);

            if(!FIRSTCHILD(node) && type != VOID_TYPE){
                fprintf(stderr, "line %d: error return type ", node->lineno);
                fprintf(stderr, "\033[1m'%s'\033[0m expected\n", type == INT ? "int" : (type == CHAR ? "char" : "void"));
                err = 1;
            }

            if(FIRSTCHILD(node) && type == VOID_TYPE){
                fprintf(stderr, "line %d: error returning type ", node->lineno);
                fprintf(stderr, "\033[1m'%s'\033[0m no value was expected for type \033[1m'void'\033[0m\n", type == INT ? "int" : (type == CHAR ? "char" : "void"));
                err = 1;
            }
            break;

		case Pointer:
			if (FIRSTCHILD(node)->kind == Identifier){
				strcpy(id, FIRSTCHILD(node)->u.identifier);
			}

			else { //ValueOf
				strcpy(id, FIRSTCHILD(FIRSTCHILD(node))->u.identifier);			
			}
			
			if (!identifierIsPointer(symtab, id)){
                fprintf(stderr, "line %d: ", node->lineno);
                fprintf(stderr, "\033[1;31merror:\033[0m invalid type argument of unary ‘*’\n");
			}

			
			break;

		default:
			break;
	}

	return;
}


int lookup_types(char* id, int* types, int* isPointer){

	Node* program = root;
	Node* funcdeclist = SECONDCHILD(program);
	Node* funcdec = FIRSTCHILD(funcdeclist);
	Node* id_node;
	Node* paramlist;
	Node* param;
	Node* type;
	char* id_tmp;
	int i = 0;
	
	while (funcdec) {
	
		//if function's type is void
		if (FIRSTCHILD(funcdec)->kind == Identifier){
			id_node = FIRSTCHILD(funcdec);
			paramlist = SECONDCHILD(funcdec);
		}
		
		else {
			id_node = SECONDCHILD(funcdec);
			paramlist = THIRDCHILD(funcdec);
		}
		
		id_tmp = id_node->u.identifier;
	
		//if we have found the right function declaration 
		if (!strcmp(id, id_tmp)){
			param = FIRSTCHILD(paramlist);
			
			while (param){
				//if the parameter is a pointer
				if (FIRSTCHILD(param)->kind == Pointer){
					type = FIRSTCHILD(FIRSTCHILD(param));
					isPointer[i] = 1;
				}
				
				else {
					type = FIRSTCHILD(param);			
					isPointer[i] = 0;
				}
				//fill the types tab with the corresponding type

				types[i] = (type->kind == Int ? INT : CHAR);  
				param = param->nextSibling;
				i++;
			}
		}
		funcdec = funcdec->nextSibling;
	} 
	return i;
}

