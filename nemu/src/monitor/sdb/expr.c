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

enum {
  TK_NOTYPE = 256, TK_EQ = 133,
	
	NUM = 129, HEXNUM = 130,
	REGISTER = 131, 

	AND = '&', TK_NOTEQ = 132,
	OR = '|', NOT = '!',
	
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
	{"\\*", '*'},					// times
	{"\\/", '/'},					// divide

	{"\\(", '('},					// left bracket
	{"\\)", ')'},					// right bracket
	
	{"[1-9][0-9]*", NUM},			// decimal integer
	{"0x[0-9]*", HEXNUM}, // HEX integer
	{"$[a-z][0-9]", REGISTER}, // register

	{"&&", AND},					// and
	{"\\|\\|", OR},				// or
	{"!", '!'},						// not
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
int eval(int p, int q)__attribute((unused));

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
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
				printf("type=%d\n",rules[i].token_type);
        switch (rules[i].token_type) {
					case '+': case '-': case '*': case '/': case '(': case ')':
						tokens[nr_token++].type = rules[i].token_type;break;
					case NUM:
						tokens[nr_token].type = rules[i].token_type; strncpy(tokens[nr_token++].str, substr_start, substr_len);break;
          default: continue;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
	/*for (i = 0; i < nr_token; i++){
		printf("%d, %s\n", tokens[i].type, tokens[i].str);
	}*/
  return true;
}

int expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
	
  /* TODO: Insert codes to evaluate the expression. */
	printf("%d\n", eval(0, nr_token-1));
  return 0;
}

int eval(int p, int q) {
	printf("%d,%d\n", p, q);
	if (p > q) {
		/* Bad expression */
		assert(0);
	}
	else if (p == q) {
		switch (tokens[p].type) {
			case NUM: case HEXNUM: return atoi(tokens[p].str);
			case REGISTER: return atoi(tokens[p].str);
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
		if (op == -1) assert(0);
		int val1 = eval(p, op - 1);
		int val2 = eval(op + 1, q);
		printf("val1=%d val2=%d\n", val1, val2);
		switch (tokens[op].type) {
			case '+': return val1 + val2;
			case '-': return val1 - val2;
			case '*': return val1 * val2;
			case '/': return val1 / val2;
			default: assert(0);
		}
	}
	return 0;
}


static int find_op(int p, int q){
	int i;
	for (i = q; i >= p; i--) {
		if (tokens[i].type == '(' || tokens[i].type == ')' || tokens[i].type == NUM || tokens[i].type == HEXNUM || tokens[i].type == REGISTER) continue;
		if (tokens[i].type == '+' || tokens[i].type == '-') {
			return i;
		}
		if (tokens[i].type == '*' || tokens[i].type == '/') {
			return i;
		}
	}
	return -1;
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
