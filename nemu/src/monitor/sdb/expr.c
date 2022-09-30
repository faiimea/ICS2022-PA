/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <memory/paddr.h>
enum {
  TK_NOTYPE = 256, TK_EQ = 255,
	
	NUM = 254, HEXNUM = 253,
	REGISTER = 252, NEG = 251,
	DEREF = 250,

	AND = 249, TK_NOTEQ = 248,
	OR = 247, NOT = '!',
	LE = 246, BE = 245,
	LESS = 244, BIGGER = 243,
	LM = 242, RM = 241,
	
	PLUS = '+', MINUS = '-',
	TIMES = '*', DIVIDE = '/',
	LEFT_BRACKET = '(', RIGHT_BRACKET = ')', 
  /* TODO: Add more token types */
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
	{"!=", TK_NOTEQ},		  // not equal
	{"\\-", '-'},					// minus
	{"\\*", '*'},				// times
	{"\\/", '/'},					// divide

	{"\\(", '('},					// left bracket
	{"\\)", ')'},					// right bracket
	
	{"0[x,X][0-9,a-f]+", HEXNUM}, // HEX integer
	{"[0-9]+", NUM},			// decimal integer
	{"\\$[a-z, 0-9]+", REGISTER}, // register

	{"&&", AND},					// and
	{"\\|\\|", OR},				// or
	{"!", '!'},						// not
	{"<=", LE},						// less equal
	{">=", BE},						// bigger equal
	{"<", LESS},					// less
	{">", BIGGER},				// bigger
	{"<<", LM},						// left move
	{">>", RM},						// right move
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;
static bool check_parentheses(int p, int q)__attribute__((unused));
static int find_op(int p, int q)__attribute__((unused));
word_t eval(int p, int q)__attribute((unused));

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;
	memset(tokens, 0, sizeof(Token)*32);
  while (e[position] != '\0') {
		//printf("e=%s\n", e);
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
				if (rules[i].token_type == TK_NOTYPE) break;
				tokens[nr_token].type = rules[i].token_type;
        switch (rules[i].token_type) {
					case '+': case '-': case '*': case '/': case '(': case ')':
						nr_token++;break;
					case NUM:
						strncpy(tokens[nr_token++].str, substr_start, substr_len);break;
					case HEXNUM:
						strncpy(tokens[nr_token++].str, substr_start, substr_len);break;
					case REGISTER:
						//printf("tail=%s\n", substr_start);
						strncpy(tokens[nr_token++].str, substr_start, substr_len);break;
          default: nr_token++;break;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
	for (i = 0; i < nr_token; i++) {
		if (tokens[i].type == '-' && (i == 0 || (tokens[i-1].type != ')' && tokens[i-1].type != NUM))) {
			tokens[i].type = NEG;
		}

		if (tokens[i].type == '*' && (i == 0 || (tokens[i-1].type != ')' && tokens[i-1].type != NUM))) {
			tokens[i].type = DEREF;
		}
	}
	/*for (i = 0; i < nr_token; i++) {
		if (tokens[i].type == NUM)
			printf("num=%s\n", tokens[i].str);
	}*/
  return true;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
	
  /* TODO: Insert codes to evaluate the expression. */
	printf("%d\n", eval(0, nr_token-1));
  return 0;
}

word_t eval(int p, int q) {
	if (p > q) {
		/* Bad expression */
		assert(0);
		//return -1;
	}
	else if (p == q) {
		bool flag=false;
		bool *success = &flag;
		int res;
		switch (tokens[p].type) {
			case NUM: return atoi(tokens[p].str);
			case HEXNUM: sscanf(tokens[p].str, "%x", &res);return res;
			case REGISTER: return isa_reg_str2val(tokens[p].str, success);
		}
		/* Single token.
		 * For now this token should be a number.
		 * Return the value of the number.
		 */
	}
	else if (check_parentheses(p, q) == true) {
		/* The expression is surrounded by a matched pair of parentheses.
		* If that is the case, just throw away the parentheses.
		*/
			return eval(p + 1, q - 1);
	}
	else {
		int op;
		op = find_op(p, q);
		//printf("op=%d\n", op);
		if (op == -1) assert(0);
		
		//some special operators
		if (tokens[op].type == NEG) {
			return eval(op + 1, q) * (-1);
		}
		if (tokens[op].type == DEREF) {
			return paddr_read(eval(op + 1, q), 4);
		}
		if (tokens[op].type == '!') {
			return !eval(op+1, q);
		}

		word_t val1 = eval(p, op - 1);
		word_t val2 = eval(op + 1, q);

		//printf("val1=%x val2=%x\n", val1, val2);
		switch (tokens[op].type) {
			case '+': return val1 + val2;
			case '-': return val1 - val2;
			case '*': return val1 * val2;
			case '/': return val1 / val2;
			case AND: return val1 && val2;
			case OR: return val1 || val2;
			case TK_EQ: return val1 == val2;
			case TK_NOTEQ: return val1 != val2;
			case LM: return (val1 << val2);
			case RM: return (val1 >> val2);
			case LESS: return val1 < val2;
			case BIGGER: return val1 > val2;
			case LE: return val1 <= val2;
			case BE: return val1 >= val2;
			default: assert(0);
		}
	}
	return 0;
}

static int priority(int sign) {
	switch (sign) {
		case NEG: case DEREF: return 8;
		case '+': case '-': return 6;
		case '*': case '/': return 7;
		case TK_NOTEQ: case TK_EQ: return 3;
		case AND: return 2;
		case OR: return 1;
		case LE: case BE: case LESS: case BIGGER: return 4;
		case LM: case RM: return 5;
	}
	return 100;
}


static int find_op(int p, int q){
	int i;
	int op = -1;
	int cnt = 0;

	for (i = p; i <= q; i++) {
		//printf("%d.type =%d\n", i, tokens[i].type);
		if (tokens[i].type == NUM || tokens[i].type == HEXNUM || tokens[i].type == REGISTER) continue;
		if (tokens[i].type == '(') {
			cnt++;
		}
		else if (tokens[i].type == ')') {
			cnt--;
		}
		else if (priority(tokens[i].type) <= priority(tokens[op].type) && cnt == 0) {
			//printf("%d\n", i);
			op = i;
		}
	}
	return op;
}

static bool check_parentheses(int p, int q){
		int i;
		int cnt = 0;
		if (tokens[p].type != '(' || tokens[q].type != ')')
			return false;
		for (i = p; i <= q; i++) {
			if (tokens[i].type == '(') {
				cnt++;
			}
			if (tokens[i].type == ')') {
				cnt--;
			}
			if (cnt == 0 && i < q) {
				return false;
			}
		}
		if (cnt != 0) {
			return false;
		}
		return true;
}
