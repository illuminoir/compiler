#include "translate.h"

/* counters for the labels in control structures */
int label_counter = 0;
int label_else = 0;
int label_while = 0;
int label_order = 0;

/* variable for '!' operator */
int var_neg = 0;

/* boolean to check whether a function call argument is global or local */
int is_global = 0;

/* boolean to check whether the function has a return instruction */
int funcHasReturn = 0;


void getAddrVar(SymbolTable *symtab, char* ident, FILE* file){
	char addrAccess[128];
	char line[128];
	ListEntry* tmp = symtab->list;

	//search for variable in current function table
	while(tmp){
		if(!strcmp(tmp->identifier, ident)){		
			sprintf(addrAccess, "	mov rax, rbp			; ┐ Identifier: %s\n", ident);
			generate(file, addrAccess); //address is found
			sprintf(line, "	sub rax, %d				; |\n", tmp->address + 8);
			generate(file, line);
			return;
		}
		tmp = tmp->next;
	}

	//search for variable in parent table
	tmp = symtab->parent->list;

	while(tmp){
		if(!strcmp(tmp->identifier, ident)){
			sprintf(addrAccess, "	mov rax, rbp			; ┐ Identifier: %s\n", ident);
			generate(file, addrAccess); //address is found
			sprintf(line, "	sub rax, %d				; |\n", tmp->address);
			generate(file, line);
			return;
		}
		tmp = tmp->next;
	}

	//variable has no address (function name)
	return;
}

int getAddrParam(SymbolTable *symtab, char* ident){
	char addrAccess[128];
	ListEntry* tmp = symtab->list;

	//search for variable in current function table
	while(tmp){
		if(!strcmp(tmp->identifier, ident)){
			return tmp->address + 8;
		}
		tmp = tmp->next;
	}

	tmp = symtab->parent->list;

	//search for variable in parent table
	while(tmp){
		if(!strcmp(tmp->identifier, ident)){
			is_global = 1;		
			return tmp->address + 8;
		}
		tmp = tmp->next;
	}
	
	return -1;
}

void generate(FILE* file, char* text){
	fputs(text, file);
}

void update_neg(){
	if(var_neg == 1)
		var_neg = 0;
	else
		var_neg = 1;
}


void translate(Node *node, FILE *file, SymbolTable *symtab){
	char tmp[32];
	char line[256];
	Node* node_tmp;

	int counter;
	
	int cpt; //counter of parameters in function declaration
	char id[128];
	char reg[4];

	Node* paramlist;
	Node* body;
	Node* ret;
	
	int type;
	
	int addr;

	if(!node)
		return;

	switch(node->kind){
		case Program :
			/* header for NASM code */
			sprintf(line, "	resb %3d 				; |\n", symtab->counter);
			generate(file, "global _start				; ┐ Program\n");
			generate(file, "extern printf				; |\n");
			generate(file, "extern scanf				; |\n");
			generate(file, "section .bss				; |\n");
			generate(file, "__static:					; |\n");
			generate(file, line);
			generate(file, "section .data				; |\n");
			generate(file, "__format_int:				; |\n");
			generate(file, "	db \"%d\", 10, 0			; |\n");
			generate(file, "__format_char:				; |\n");
			generate(file, "	db \"%c\", 10, 0			; |\n");
			generate(file, "__format_reade:				; |\n");
			generate(file, "	db \"%d\", 0				; |\n");
			generate(file, "__format_readc:				; |\n");
			generate(file, "	db \"%c\", 0				; |\n");
			generate(file, "section .text				; |\n");
			generate(file, "_start:						; |\n");
			generate(file, "	call main				; |\n");
			generate(file, "	mov rax, 60				; |\n");
			generate(file, "	mov rdi, 0				; |\n");
			generate(file, "	syscall					; |\n");

			/* ignore global variables and write the code for the functions */
			translate(SECONDCHILD(node), file, symtab); 
			break;

		case FuncDecList:
			node_tmp = FIRSTCHILD(node);
			translate(node_tmp, file, symtab);  //translate the first statement
			while(node_tmp->nextSibling){ //translate all the other statements
				translate(node_tmp->nextSibling, file, symtab);
				node_tmp = node_tmp->nextSibling;
			}
			break;

		case FuncDec:
			//if the function is main
			if (!strcmp(node->st->name, "main")){
				sprintf(tmp, "%s", node->st->name);
				sprintf(line, "%s:						; ┐ FuncDec[begin] : %s\n", tmp, tmp);
				generate(file, line);
				generate(file, "	mov rbp, rsp			; |\n");
				translate(FOURTHCHILD(node), file, node->st); //body
				sprintf(line, "    mov rsp, rbp			; ┐ FuncDec[end]: %s\n", tmp);
				generate(file, line);
				generate(file, "	ret 					; |\n");
				
			}
			
			else {
				sprintf(tmp, "%s", node->st->name);
				sprintf(line, "%s:						; ┐ FuncDec[begin] : %s\n", tmp, tmp);
				generate(file, line);

				// If function returns void
				if(FIRSTCHILD(node) -> kind == Identifier){
					paramlist = SECONDCHILD(node);
					body = THIRDCHILD(node);
				}
				
				else {
					paramlist = THIRDCHILD(node);
					body = FOURTHCHILD(node);
				}
				
				//bloc d'activation
				generate(file, "	push rbp 				; | bloc d'activation\n");
				generate(file, "	mov rbp, rsp			; |\n");
				
				//get all the parameters
				translate(paramlist, file, node->st);
				
				//body of the function
				translate(body, file, node->st);
				
				//return 		
				if(!funcHasReturn){
					//align stack
					sprintf(line, "	add rsp, %d				; | VarDecList\n", node->st->counter);
					generate(file, line);	
					generate(file, "	pop rbp					; |\n");
					generate(file, "	ret						; |\n");
				}

				funcHasReturn = 0;

			}
			break;

		case ParamList:
			node_tmp = FIRSTCHILD(node);
			cpt = 0; 

			while (node_tmp) {
				strcpy(id, SECONDCHILD(node_tmp)->u.identifier);		
				switch(cpt) {
					case 0 : strcpy(reg, "rdi");
							 break;
					case 1 : strcpy(reg, "rsi");
							 break;
					case 2 : strcpy(reg, "rdx");
							 break;
					case 3 : strcpy(reg, "rcx");
							 break;
					case 4 : strcpy(reg, "r8d");
							 break;
					case 5 : strcpy(reg, "r9d");
							 break;
					default :
						//mettre sur la pile
							 break;
				}
				
				if (FIRSTCHILD(node_tmp)->kind == Pointer) {
					
				} 

				else if (FIRSTCHILD(node_tmp)->kind == Deref){
				
				}
				
				else {
					sprintf(line, "	mov [rbp - %d], %s		; | Param\n", getAddrParam(symtab, id), reg);
					generate(file, line);
				}
				cpt++;
				node_tmp = node_tmp->nextSibling;
			}
			break;			

		case Body:
			translate(FIRSTCHILD(node), file, symtab); //declaration of variables
			translate(SECONDCHILD(node), file, symtab); //instructions
			break;
	
		case Return:
			ret = FIRSTCHILD(node);
			if (ret){
				if (ret->kind == IntLiteral){
					sprintf(line, "	mov rax, %d				; ┐ Return integer %d\n", ret->u.integer, ret->u.integer);
					generate(file, line); 						
				}
				else if (ret->kind == CharLiteral){
					sprintf(line, "	mov rax, %d				; ┐ Returning character %c\n", ret->u.character, ret->u.character);
					generate(file, line); 					
				}
				else if (ret->kind == ValueOf){
					//recupérer la position sur la pile
					strcpy(id, FIRSTCHILD(ret) -> u.identifier);
					sprintf(line, "	mov rax, [rbp - %d]		; ┐ Returning identifier %s\n",  getAddrParam(symtab, id), id);
					generate(file, line); 					
				}
				
				//return node has no child, return nothing
				else {
					translate(FIRSTCHILD(node), file, symtab);
					generate(file, "	pop rax					; |\n");
				}
			}

			//if we're returning outside of main function, remove the stack block before leaving the function
			if(strcmp(symtab->name, "main")){
				sprintf(line, "	add rsp, %d				; | VarDecList\n", symtab->counter);
				generate(file, line);
				generate(file, "	pop rbp 				; ┐ Return\n");
				generate(file, "	ret 					; | \n");
			}

			funcHasReturn = 1;
			
			break;

		case VarDeclList:		
			sprintf(line, "	sub rsp, %d				; | VarDecList\n", symtab->counter);
			generate(file, line);
			translate(FIRSTCHILD(node), file, symtab);
			break;

		case Int:
			translate(FIRSTCHILD(node), file, symtab); //vardec
			translate(node->nextSibling, file, symtab); //if char variables are declared
			break;

		case Char:
			translate(FIRSTCHILD(node), file, symtab); //vardec
			translate(node->nextSibling, file, symtab); //if int variables are declared
			break;

		case VarDec:
			translate(node->nextSibling, file, symtab);
			break;

		case StmtList:
			node_tmp = FIRSTCHILD(node);
			translate(node_tmp, file, symtab);  //translate the first statement
			while(node_tmp->nextSibling){ //translate all the other statements
				translate(node_tmp->nextSibling, file, symtab);
				node_tmp = node_tmp->nextSibling;
			}
			break;

		case Pointer:
			
			if (FIRSTCHILD(node)->kind == Identifier){
				strcpy(id, FIRSTCHILD(node)->u.identifier);
			}
			else {   //ValueOf
				strcpy(id, FIRSTCHILD(FIRSTCHILD(node))->u.identifier);
			}
			getAddrVar(symtab, id, file);
			generate(file, "	push rax				; |\n");
			
			generate(file, "	pop rax					; ┐ ValueOf(addr)\n");
			generate(file, "	push qword [rax]		; |\n");
			
			sprintf(line, "	pop rbx					; ┐ address stored in %s\n", id);
			generate(file, line);
			
			generate(file, "	mov rax, rbp			; ┐\n");
			generate(file, "	sub rax, rbx			; |\n");
			generate(file, "	push rax				; |\n");
			
			if (FIRSTCHILD(node)->kind == ValueOf){
				generate(file, "	pop rax					; ┐ ValueOf(addr)\n");
				generate(file, "	push qword [rax]		; |\n");
			}

			break;

		case Deref:
			strcpy(id, FIRSTCHILD(FIRSTCHILD(node))->u.identifier);
			sprintf(line, "	push %d					; ┐ Deref\n", getAddrParam(symtab, id));
			generate(file, line);
			break;

		case Assign:
			translate(FIRSTCHILD(node), file, symtab);
			translate(SECONDCHILD(node), file, symtab);
			generate(file, "	pop rbx					; ┐ Assign\n");
			generate(file, "	pop rax					; |\n");
			generate(file, "	mov qword [rax], rbx	; |\n");
			break;
			
		case Identifier:
			getAddrVar(symtab, node->u.identifier, file);
			generate(file, "	push rax				; |\n");
			break;
			
		case IntLiteral:
			sprintf(line, "    push %d 					; ┐ IntLiteral: %d\n", node->u.integer, node->u.integer);
			generate(file, line);
			break;
			
		case CharLiteral:
			sprintf(line, "    push %d 				    ; ┐ CharLiteral: %c\n", node->u.character, node->u.character);
			generate(file, line);
			break;

		case Add:
			translate(FIRSTCHILD(node), file, symtab); //left value of add
			translate(SECONDCHILD(node), file, symtab); //right value of add

			generate(file, "	pop rbx					; ┐ Add\n");
			generate(file, "	pop rax					; |\n");
			generate(file, "	add rax, rbx			; |\n");
			generate(file, "	push rax				; |\n");
			break;

		case Sub:
			translate(FIRSTCHILD(node), file, symtab); //left value of sub
			translate(SECONDCHILD(node), file, symtab); //right value of sub

			generate(file, "	pop rbx					; ┐ Sub\n");
			generate(file, "	pop rax					; |\n");
			generate(file, "	sub rax, rbx			; |\n");
			generate(file, "	push rax				; |\n");
			break;

		case Mult:
			translate(FIRSTCHILD(node), file, symtab); //left value of mult
			translate(SECONDCHILD(node), file, symtab); //right value of mult

			generate(file, "	pop rbx					; ┐ Mult\n");
			generate(file, "	pop rax					; |\n");
			generate(file, "	imul rax, rbx			; |\n");
			generate(file, "	push rax				; |\n");
			break;

		case Div:
			translate(FIRSTCHILD(node), file, symtab); //left value of div
			translate(SECONDCHILD(node), file, symtab); //right value of div

			generate(file, "	pop rbx					; ┐ Div\n");
			generate(file, "	pop rax					; |\n");
			generate(file, "	xor rdx, rdx			; |\n");
			generate(file, "	idiv rbx				; |\n");
			generate(file, "	push rax				; |\n");

			break;
			
		case Modulo:
			translate(FIRSTCHILD(node), file, symtab); //left value of modulo
			translate(SECONDCHILD(node), file, symtab); //right value of modulo

			generate(file, "	pop rbx					; ┐ Modulo\n");
			generate(file, "	pop rax					; |\n");
			generate(file, "	xor rdx, rdx			; |\n");
			generate(file, "	idiv rbx				; |\n");
			generate(file, "	push rdx				; |\n");

			break;

		case Print:
            translate(FIRSTCHILD(node), file, symtab); //identifier that is printed

            if(FIRSTCHILD(node)->kind == Pointer)
            	node_tmp = FIRSTCHILD(FIRSTCHILD(FIRSTCHILD(node)));
            else
            	node_tmp = FIRSTCHILD(FIRSTCHILD(node));

            if(FIRSTCHILD(node)->kind == CharLiteral)
                type = 256;
            else if(FIRSTCHILD(node)->kind == IntLiteral)
                type = 255;
            else
                type = getTypeVar(symtab, node_tmp->u.identifier);

            if(type != 256) //type is int
                generate(file, "    mov rdi, __format_int   ; ┐ Print\n");
            else //type is char otherwise
                generate(file, "    mov rdi, __format_char  ; ┐ Print\n");
            generate(file, "    pop rsi                 ; |\n");
            generate(file, "    xor rax, rax            ; |\n");
            generate(file, "    call printf             ; |\n");
            break;

		case AddressOf:
			translate(FIRSTCHILD(node), file, symtab); //identifier
			generate(file, "	pop rax					; ┐ AddressOf\n");
			generate(file, "	push rax				; |\n");
			break;

		case Reade:
			translate(FIRSTCHILD(node), file, symtab); //identifier that is printed
			generate(file, "	mov rdi, __format_reade	; ┐ Reade\n");
			generate(file, "	pop rsi 				; |\n");
			generate(file, "	xor rax, rax			; |\n");
			generate(file, "	call scanf 				; |\n");
			break;

		case Readc:
			translate(FIRSTCHILD(node), file, symtab); //identifier that is printed
			generate(file, "	mov rdi, __format_readc	; ┐ Readc\n");
			generate(file, "	pop rsi 				; |\n");
			generate(file, "	xor rax, rax			; |\n");
			generate(file, "	call scanf 				; |\n");
			break;

			
		case FuncCall:
			//translate the arguments
			translate(SECONDCHILD(node), file, symtab);
			
			sprintf(line, "	call %s 			; ┐ FuncCall %s\n", FIRSTCHILD(node)->u.identifier, FIRSTCHILD(node)->u.identifier);
			generate(file, line);
			
			if (getTypeVar(symtab->parent, FIRSTCHILD(node)->u.identifier) != VOID_TYPE) 
				generate(file, "	push rax				; |\n");
			break;

		case Arguments:
			cpt = 0; 
			node_tmp = FIRSTCHILD(node); //ValueOf or Deref or Pointer or IntLiteral or CharLiteral
			while (node_tmp) {
				if(node_tmp->kind == ValueOf)
					strcpy(id, FIRSTCHILD(node_tmp)->u.identifier);		
				else if (node_tmp->kind == Deref || node_tmp->kind == Pointer){
					strcpy(id, FIRSTCHILD(FIRSTCHILD(node_tmp))->u.identifier);
					addr = getAddrParam(symtab, id);
				}
				switch(cpt) {
					case 0 : strcpy(reg, "rdi");
							 if(node_tmp->kind == ValueOf)
							 	addr = getAddrParam(symtab, id);							 
							 break;
					case 1 : strcpy(reg, "rsi");
							 break;
					case 2 : strcpy(reg, "rdx");
							 break;
					case 3 : strcpy(reg, "rcx");
							 break;
					case 4 : strcpy(reg, "r8d");
							 break;
					case 5 : strcpy(reg, "r9d");
							 break;
					default :
						//mettre sur la pile
							 break;
				}

				if(node_tmp->kind == ValueOf)
					addr = getAddrParam(symtab, id);

				if(node_tmp->kind == IntLiteral || node_tmp->kind == Deref)
					sprintf(line, " 	mov %s, %d 				; | Argument\n", reg, (node_tmp->kind == IntLiteral) ? node_tmp->u.integer : addr);
				else if(node_tmp->kind == CharLiteral)
					sprintf(line, " 	mov %s, %c 				; | Argument\n", reg, node_tmp->u.character);
				else {
					if(is_global)
						sprintf(line, "	mov %s, [__static + %d]	; | Argument\n", reg, addr - 8);
					else
						sprintf(line, "	mov %s, [rbp - %d]		; | Argument\n", reg, addr);
				}
				generate(file, line);
				cpt++;
				node_tmp = node_tmp->nextSibling;

				is_global = 0;
			}
			//generate(file, line);
			
			break;

		case ValueOf:
			translate(FIRSTCHILD(node), file, symtab); //identifier
			generate(file, "	pop rax					; ┐ ValueOf\n");
			generate(file, "	push qword [rax]		; |\n");		
			break;
			
		case Or:
			translate(FIRSTCHILD(node), file, symtab);
			
			generate(file, "	pop rax					; ┐ Or[begin]\n");
			generate(file, "	cmp rax, 0				; |\n");
			
			sprintf(line, "    jne __label_%d			; |\n", label_counter);
			generate(file, line);
			
			translate(SECONDCHILD(node), file, symtab);
			
			generate(file, "	pop rax					; ┐ Or[end]\n");
			generate(file, "	cmp rax, 0				; |\n");
			
			sprintf(line, "    jne __label_%d			; |\n", label_counter);
			generate(file, line);
			
			if(var_neg)
				generate(file, "	push 1					; |\n");
			else
				generate(file, "	push 0					; |\n");

			sprintf(line, "    jmp __label_%d			; |\n", label_counter+1);
			generate(file, line);			

			sprintf(line, "__label_%d:					; |\n", label_counter);
			generate(file, line);

			if(var_neg)
				generate(file, "	push 0					; |\n");
			else
				generate(file, "	push 1					; |\n");
			
			sprintf(line, "__label_%d:					; |\n", ++label_counter);
			generate(file, line);
			label_counter++;		
			break;			
			
		case And:
			translate(FIRSTCHILD(node), file, symtab);
			
			generate(file, "	pop rax					; ┐ And[begin]\n");
			generate(file, "	cmp rax, 0				; |\n");
			
			sprintf(line, "    je __label_%d			; |\n", label_counter);
			generate(file, line);
			
			translate(SECONDCHILD(node), file, symtab);
			
			generate(file, "	pop rax					; ┐ And[end]\n");
			generate(file, "	cmp rax, 0				; |\n");
			
			sprintf(line, "	je __label_%d				; |\n", label_counter);
			generate(file, line);
			
			if(var_neg)
				generate(file, "	push 0					; |\n");
			else
				generate(file, "	push 1					; |\n");

			sprintf(line, "    jmp __label_%d			; |\n", label_counter+1);
			generate(file, line);			

			sprintf(line, "__label_%d:					; |\n", label_counter);
			generate(file, line);

			if(var_neg)
				generate(file, "	push 1					; |\n");
			else
				generate(file, "	push 0					; |\n");
			
			sprintf(line, "__label_%d:					; |\n", ++label_counter);
			generate(file, line);
			label_counter++;
			break;

		case IfThen:
			//first child is the condition
			translate(FIRSTCHILD(node), file, symtab);
			
			counter = label_counter;			
			
			//check condition			
			generate(file, "	pop rax					; ┐ IfThen[begin]\n");
			generate(file, "	cmp rax, 0				; | \n");	
					
			//jump if condition false
			sprintf(line, "    je __label_%d			; |\n", counter);
			generate(file, line);
			label_counter++;
			//code if condition true
			translate(SECONDCHILD(node), file, symtab);

			sprintf(line, "__label_%d:					; |\n", counter);
			generate(file, line);	
			
			label_counter+=1;	
					
			break;

		case IfThenElse:
			//first child is the condition
			counter = label_else ;
			translate(FIRSTCHILD(node), file, symtab);
			//check condition			
			generate(file, "	pop rax					; ┐ IfThenElse[begin]\n");
			generate(file, "	cmp rax, 0				; | \n");
			
			//jump if condition false
			sprintf(line, "    je __label_else_%d		; |\n", counter);
			generate(file, line);
			//code if condition true
			translate(SECONDCHILD(node), file, symtab);
			sprintf(line, "    jmp __label_else_%d 		; ┐ IfThenElse[mid]\n", counter+1);
			generate(file, line);

			sprintf(line, "__label_else_%d:				; |\n", counter);
			generate(file, line);

			//code if condition false
			label_else+=2;
			translate(THIRDCHILD(node), file, symtab);

			sprintf(line, "__label_else_%d:				; ┐ IfThenElse[end]\n", counter+1);
			generate(file, line);

			break;

		case While:
			counter = label_while;
			//fisrt child is condition
			translate(FIRSTCHILD(node), file, symtab);
		
			sprintf(line, "__label_while_%d:			; ┐ While[begin]\n", counter);
			generate(file, line);	
			
			//check condition			
			generate(file, "	pop rax					; ┐ While[mid]\n");
			generate(file, "	cmp rax, 0				; | \n");
			
			//exit loop if condition false
			sprintf(line, "    je __label_while_%d		; |\n", counter+1);
			generate(file, line);
			
			//code if condition true
			label_while+=2;
			translate(SECONDCHILD(node), file, symtab);
			//loop
			translate(FIRSTCHILD(node), file, symtab);
			sprintf(line, "    jmp __label_while_%d		; |\n", counter);
			generate(file, line);	
			
			sprintf(line, "__label_while_%d:			; |\n", counter+1);
			generate(file, line);	
			
			label_while++;	
				
			break;
			
		case Inf:
			translate(FIRSTCHILD(node), file, symtab);
			translate(SECONDCHILD(node), file, symtab);
			//second child
			generate(file, "	pop rbx					; ┐ Inf\n");
			//first child
			generate(file, "	pop rax					; |\n");
			generate(file, "	cmp rax, rbx			; |\n");
			
			//si firstchild >= secondchild
			sprintf(line, "    jge __label_order_%d		; |\n", label_order);
			generate(file, line);
			if(var_neg)
				generate(file, "	push 0					; |\n");
			else
				generate(file, "	push 1					; |\n");
			
			sprintf(line, "    jmp __label_order_%d		; |\n", label_order+1);
			generate(file, line);
			
			sprintf(line, "__label_order_%d:			; |\n", label_order);
			generate(file, line);
			
			if(var_neg)
				generate(file, "	push 1					; |\n");
			else
				generate(file, "	push 0					; |\n");
			
			sprintf(line, "__label_order_%d:			; |\n", label_order+1);
			generate(file, line);			
			
			label_order+=2;
			
			break;
			
		case Sup:
			translate(FIRSTCHILD(node), file, symtab);
			translate(SECONDCHILD(node), file, symtab);
			//second child
			generate(file, "	pop rbx					; ┐ Sup\n");
			//first child
			generate(file, "	pop rax					; |\n");
			generate(file, "	cmp rax, rbx			; |\n");
			
			//si firstchild <= secondchild
			sprintf(line, "    jle __label_order_%d		; |\n", label_order);
			generate(file, line);
			if(var_neg)
				generate(file, "	push 0					; |\n");
			else
				generate(file, "	push 1					; |\n");
			
			sprintf(line, "    jmp __label_order_%d		; |\n", label_order+1);
			generate(file, line);
			
			sprintf(line, "__label_order_%d:			; |\n", label_order);
			generate(file, line);
			
			if(var_neg)
				generate(file, "	push 1					; |\n");
			else
				generate(file, "	push 0					; |\n");
			
			sprintf(line, "__label_order_%d:			; |\n", label_order+1);
			generate(file, line);			
			
			label_order+=2;
			
			break;	
			
		case InfEq:
			translate(FIRSTCHILD(node), file, symtab);
			translate(SECONDCHILD(node), file, symtab);
			//second child
			generate(file, "	pop rbx					; ┐ InfEq\n");
			//first child
			generate(file, "	pop rax					; |\n");
			generate(file, "	cmp rax, rbx			; |\n");
			
			//si firstchild > secondchild
			sprintf(line, "    jg __label_order_%d		; |\n", label_order);
			generate(file, line);
			if(var_neg)
				generate(file, "	push 0					; |\n");
			else
				generate(file, "	push 1					; |\n");
			
			sprintf(line, "    jmp __label_order_%d		; |\n", label_order+1);
			generate(file, line);
			
			sprintf(line, "__label_order_%d:			; |\n", label_order);
			generate(file, line);
			
			if(var_neg)
				generate(file, "	push 1					; |\n");
			else
				generate(file, "	push 0					; |\n");
			
			sprintf(line, "__label_order_%d:			; |\n", label_order+1);
			generate(file, line);			
			
			label_order+=2;
			
			break;	
			
		case SupEq:
			translate(FIRSTCHILD(node), file, symtab);
			translate(SECONDCHILD(node), file, symtab);
			//second child
			generate(file, "	pop rbx					; ┐ SupEq\n");
			//first child
			generate(file, "	pop rax					; |\n");
			generate(file, "	cmp rax, rbx			; |\n");
			
			//si firstchild < secondchild
			sprintf(line, "    jl __label_order_%d		; |\n", label_order);
			generate(file, line);
			if(var_neg)
				generate(file, "	push 0					; |\n");
			else
				generate(file, "	push 1					; |\n");
			
			sprintf(line, "    jmp __label_order_%d		; |\n", label_order+1);
			generate(file, line);
			
			sprintf(line, "__label_order_%d:			; |\n", label_order);
			generate(file, line);
			
			if(var_neg)
				generate(file, "	push 1					; |\n");
			else
				generate(file, "	push 0					; |\n");
			
			sprintf(line, "__label_order_%d:			; |\n", label_order+1);
			generate(file, line);			
			
			label_order+=2;
			
			break;					

		case Eq:
			translate(FIRSTCHILD(node), file, symtab);
			translate(SECONDCHILD(node), file, symtab);
			//second child
			generate(file, "	pop rbx					; ┐ Eq\n");
			//first child
			generate(file, "	pop rax					; |\n");
			generate(file, "	cmp rax, rbx			; |\n");
			
			//si firstchild != secondchild
			sprintf(line, "    jne __label_order_%d		; |\n", label_order);
			generate(file, line);
			if(var_neg)
				generate(file, "	push 0					; |\n");
			else
				generate(file, "	push 1					; |\n");
			
			sprintf(line, "    jmp __label_order_%d		; |\n", label_order+1);
			generate(file, line);
			
			sprintf(line, "__label_order_%d:			; |\n", label_order);
			generate(file, line);
			
			if(var_neg)
				generate(file, "	push 1					; |\n");
			else
				generate(file, "	push 0					; |\n");
			
			sprintf(line, "__label_order_%d:			; |\n", label_order+1);
			generate(file, line);			
			
			label_order+=2;
			
			break;
			
		case Diff:
			translate(FIRSTCHILD(node), file, symtab);
			translate(SECONDCHILD(node), file, symtab);
			//second child
			generate(file, "	pop rbx					; ┐ Diff\n");
			//first child
			generate(file, "	pop rax					; |\n");
			generate(file, "	cmp rax, rbx			; |\n");
			
			//si firstchild == secondchild
			sprintf(line, "    je __label_order_%d		; |\n", label_order);
			generate(file, line);
			if(var_neg)
				generate(file, "	push 0					; |\n");
			else
				generate(file, "	push 1					; |\n");
			
			sprintf(line, "    jmp __label_order_%d		; |\n", label_order+1);
			generate(file, line);
			
			sprintf(line, "__label_order_%d:			; |\n", label_order);
			generate(file, line);
			
			if(var_neg)
				generate(file, "	push 1					; |\n");
			else
				generate(file, "	push 0					; |\n");
			
			sprintf(line, "__label_order_%d:			; |\n", label_order+1);
			generate(file, line);			
			
			label_order+=2;
			
			break;

		case Neg:
			update_neg();

			translate(FIRSTCHILD(node), file, symtab);

			update_neg();

			break;		

		default:
			break;
	}

	return;
}
