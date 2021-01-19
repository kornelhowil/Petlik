#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
/* Ilośc zmiennych */
#define NO_VAR 26
#define BASE 10000
/* Instrukcje maszyny wirtualnej */
enum { INC, ADD, CLR, JMP, DJZ, HLT };
/* Zmienna języka pętlik w systemie o podstawie 1000 */
typedef struct {
	int length;
	int *digits;
} var;
/* Instrukcja maszyny wirtualnej */
typedef struct {	
	int type;
	int arg1;
	int arg2;
} inst;
/* Funkcje kompilatora */
inst *compiler(char *petlik, int length);
bool optimal(int *i, char *petlik, int length);
int compile(int *i, int j, char *petlik, inst *m_code, int length);
/* Funkcje interpretera */
var *init();
void print(var a);
bool is_zero(var *var1);
void inc(var *var1);
void add(var *var1, var *var2);
void clr(var *var1);
void jmp(int index, int *i);
void interpreter(inst *m_code, var *variables);
/* Główny program */
int main(void)
{	
	var *variables = init();
	int c;
	while((c = getchar()) != EOF) {
		ungetc(c, stdin);
		int length; /* dlugosc kodu petlik */
		char *petlik;
	    //char *petlik = read(&length); /* wczytywanie kodu petlik */
	    getline(&petlik,(size_t*)&length,stdin);
	    if (petlik[0] != '=') {
	    	inst *m_code = compiler(petlik, length);
			interpreter(m_code, variables);
			free(m_code);
	    }
	    else
	    	print(variables[petlik[1] - 'a']);
		//free(petlik);
	}
	for (int i = 0; i < NO_VAR; i++)
		free(variables[i].digits);
	free(variables);
    return 0;
}
/* Zwraca kod programu przetłumaczony na język maszyny wirtualnej */
inst *compiler(char *petlik, int length)
{
	inst *m_code = malloc((size_t)(length + 1) * sizeof(inst));
	int index = 0;
	int end = compile(&index, 0, petlik, m_code, length);
	m_code[end].type = HLT;
	return m_code;
}
/* Sprawdza czy pętlę można zoptymalizować */
bool optimal(int *i, char *petlik, int length)
{
	int index = *i;
	char arg = petlik[index - 1];
	while (index < length) {
		if (petlik[index] == '(' || petlik[index] == arg)
			return false;
		else if (petlik[index] == ')') 
			return true;
		else
			index++;
	}
	return false;
}
/* Kompiluje kod pętlika */
int compile(int *i, int j, char *petlik, inst *m_code, int length)
{	
	while (*i < length && petlik[*i] != ')') {
		if(petlik[*i] == '(') {
			*i += 2;
			bool opt = optimal(i, petlik, length);
			if (opt == false) {
				m_code[j].type = DJZ;
				m_code[j].arg1 = petlik[*i - 1] - 'a';
				int end = compile(i, j + 1, petlik, m_code, length);
				m_code[j].arg2 = end + 1;
				m_code[end].type = JMP;
				m_code[end].arg1 = j;
				*i += 1;
				j = end + 1;
			}
			else {
				int arg2 = petlik[*i - 1];
				while (petlik[*i] != ')') {
					m_code[j].type = ADD;
					m_code[j].arg1 = petlik[*i] - 'a';
					m_code[j].arg2 = arg2 - 'a'; 
					*i += 1;
					j++;
				}
				m_code[j].type = CLR;
				m_code[j].arg1 = arg2 - 'a'; 
				*i += 1;
				j++;
			}
		}
		else {
			m_code[j].type = INC;
			m_code[j].arg1 = petlik[*i] - 'a';
			*i += 1;
			j++;
		}
	}
	return j; /* Zwraca pierwszy wolny indeks */
}
/* Inicjalizuje zmienne programu w języku pętlik */
var *init()
{
	var *variables = malloc(NO_VAR * sizeof(var));
	for (int i = 0; i < NO_VAR; i++) {
		variables[i].length = 1;
		variables[i].digits = malloc(sizeof(int));
		variables[i].digits[0] = 0;
	}
	return variables;
}
/* drukuje wartość var1*/
void print(var var1)
{
	int i = var1.length - 1;
	while(i > 0 && var1.digits[i] == 0)
		i--;
	printf("%d", var1.digits[i]);
	while(i > 0) {
		i--;
		printf("%04d", var1.digits[i]);
	}
	printf("\n");
}
/* sprawdza czy zmienna jest zerem */
bool is_zero(var *var1)
{
	for (int i = 0; i < var1->length; i++)
		if (var1->digits[i] != 0)
			return false;
	return true;
}
/* zwiększa wartość zmiennej o 1 */
void inc(var *var1)
{
	int buf = 1;
	for (int i = 0; buf == 1; i++) {
		if (var1->length == i) {
			var1->length += 1;
            var1->digits = realloc(var1->digits, (size_t)var1->length * sizeof(int));
            var1->digits[i] = 0;
		}
		var1->digits[i] += buf;
		if (var1->digits[i] >= BASE)
			var1->digits[i] -= BASE;
		else
			buf = 0;
	}
}
/* dodaje var2 do var1 */
void add(var *var1, var *var2)
{	
	int buf = 0;
	for (int i = 0; i < var2->length; i++) {
		if (i == var1->length) {
			var1->length += 1;
            var1->digits = realloc(var1->digits, (size_t)var1->length * sizeof(int));
            var1->digits[i] = 0;
		}
		var1->digits[i] += (var2->digits[i] + buf);
		if (var1->digits[i] >= BASE) {
			var1->digits[i] -= BASE;
			buf = 1;
		}
		else
			buf = 0;
	}
	if (buf == 1) {
		if (var2->length == var1->length) {
			var1->length += 1;
            var1->digits = realloc(var1->digits, (size_t)var1->length * sizeof(int));
            var1->digits[var2->length] = 0;
		}
		var1->digits[var2->length] += buf;
	}
}
/* zeruje zmienną var1 */
void clr(var *var1)
{
	for (int i = 0; i < var1->length; i++)
		var1->digits[i] = 0;
}
/* skocz do indeksu index*/
void jmp(int index, int *i)
{
	*i = index;
}
/* dekrepentuj1 var lub skocz do indeksu index */
void djz(var *var1, int index, int *i)
{
	if (is_zero(var1) == false) {
		int buf = -1;
		for (int j = 0; buf == -1; j++) {
			var1->digits[j] += buf;
			if (var1->digits[j] < 0)
				var1->digits[j] += BASE;
			else
				buf = 0;
		}
		*i += 1;
	}
	else
		*i = index;
}
/* interpreter */
void interpreter(inst *m_code, var *variables)
{
	int i = 0;
	while (m_code[i].type != HLT) {
		switch (m_code[i].type) {
			case INC:
				inc(&variables[m_code[i].arg1]);
				i++;
				break;
			case ADD:
				add(&variables[m_code[i].arg1], &variables[m_code[i].arg2]);
				i++;
				break;
			case CLR:
				clr(&variables[m_code[i].arg1]);
				i++;
				break;
			case JMP:
				jmp(m_code[i].arg1, &i);
				break;
			case DJZ:
				djz(&variables[m_code[i].arg1], m_code[i].arg2, &i);
				break;
		}
	}
}








