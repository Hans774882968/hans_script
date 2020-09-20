#include <bits/stdc++.h>
#include "lexer.cpp"
using namespace std;
typedef long long LL;
#define rep(i,a,b) for(int i = (a);i <= (b);++i)
#define re_(i,a,b) for(int i = (a);i < (b);++i)
#define dwn(i,a,b) for(int i = (a);i >= (b);--i)

//词法分析时已经保证整数合法
int to_int(string d){
    int r = 0;
    if(d[0] == '0'){
        if(d.size() <= 1) return 0;
        else if(d[1] == 'b') re_(i,2,d.size()) r = 2 * r + d[i] - '0';
        else if(d[1] == 'x') re_(i,2,d.size()) r = 16 * r + ('0' <= d[i] && d[i] <= '9' ? d[i] - '0' : tolower(d[i]) - 'a');
        else re_(i,1,d.size()) r = 8 * r + d[i] - '0';
    }
    else re_(i,0,d.size()) r = 10 * r + d[i] - '0';
    return r;
}

/*自定义返回类型，使解析过程方便
不合法时返回：{false,0,0};*/
struct Ret{
    bool fl;int val;//返回值
    int ed;//终止位置(为了避免使用全局下标，同时解决通信问题)
};

class Parser{
    //已读取的情况下，期望是变量
    #define EXPECT_VAR(u) if((u).code != 0){printf("Expected variable but get token: %s\n",(u).token.c_str());return {false,0,0};}
    private:
        vector<Node> code;
        map<string,int> func;
        vector<map<string,int> > variables;
    public:
        Parser(){variables.push_back(map<string,int>());}
        Parser(vector<Node> lex_unit):code(lex_unit){variables.push_back(map<string,int>());}
        
        bool match_bracket(int st,int &ed){
            int brket = 1;ed = st + 1;
            //特意安排了这些括号相邻："(",")","[","]","{","}"
            for(;ed < code.size() && brket != 0;++ed){
                if(code[ed].code == code[st].code) brket++;
                else if(code[ed].code == code[st].code + 1) brket--;
            }
            if(brket != 0){
                printf("Bracket '%s' doesn`t match.\n",code[st].token);return false;
            }
            return true;
        }
        //全局范围，要么是函数要么是声明语句
        bool func_and_var(){
            int p = 0;
            while(p < code.size()){
                if(code[p].code != 1){
                    printf("Unsupported type at global scope: '%s'\n",code[p].token.c_str());return false;
                }
                if(code[p + 1].code != 0){
                    printf("Illegal function name: '%s'\n",code[p + 1].token.c_str());return false;
                }
                if(code[p + 2].token != "("){
                    Ret nx = declaration(p);
                    if(!nx.fl) return false;
                    p = nx.ed;
                }
                else{
                    func[code[p + 1].token] = p;
                    int p1;bool fl = match_bracket(p + 2,p1);
                    if(!fl) return fl;
                    if(code[p1].token != "{"){
                        puts("Function body should start with '{'!");
                        return false;
                    }
                    fl = match_bracket(p1,p);
                    if(!fl) return fl;
                }
            }
            return true;
        }
        //在父作用域寻找变量
        int find_var(string var){
            dwn(i,variables.size() - 1,0){
                if(variables[i].count(var)) return i;
            }
            return -1;
        }
        //p是分号的位置
        Ret parse_semicolon(int p){
            if(code[p].token != ";"){
                printf("Expected ';' but get token: '%s'\n",code[p].token.c_str());
                return {false,0,0};
            }
            ++p;return {true,0,p};
        }
        
        Ret unit0(int p){
            Node u = code[p];++p;
            if(!u.code){
                string var = u.token;int scope_ptr = find_var(var);
                if(scope_ptr == -1){
                    printf("Variable '%s' was not declared in this scope.\n",var.c_str());return {false,0,0};
                }
                return {true,variables[scope_ptr][var],p};
            }
            else if(u.code == INTCODE) return {true,to_int(u.token),p};
            else if(u.token == "+") return unit0(p);
            else if(u.token == "-"){
                Ret nx = unit0(p);
                nx.val = -nx.val;
                return nx;
            }
            else if(u.token == "!"){
                Ret nx = unit0(p);
                nx.val = !nx.val;
                return nx;
            }
            else if(u.token == "("){
                Ret nx = unit9(p);
                if(code[nx.ed].token != ")"){
                    puts("Syntax error! Wanna ')' token!");return {false,0,0};
                }
                ++nx.ed;return nx;
            }
            printf("Unknown token: %s\n",u.token.c_str());
            return {false,0,0};
        }
        Ret unit1(int p){
            Ret nx = unit0(p);
            if(!nx.fl) return {false,0,0};
            string op = code[nx.ed].token;
            while(op == "*" || op == "/" || op == "%"){
                Ret nx1 = unit0(nx.ed + 1);
                if(!nx1.fl) return {false,0,0};
                if(op == "*") nx.val *= nx1.val;
                if(op == "/") nx.val /= nx1.val;//暂不处理ZeroDivisionError
                if(op == "%") nx.val %= nx1.val;//暂不处理ZeroDivisionError
                nx.ed = nx1.ed;
                op = code[nx1.ed].token;
            }
            return nx;
        }
        Ret unit2(int p){
            Ret nx = unit1(p);
            if(!nx.fl) return {false,0,0};
            string op = code[nx.ed].token;
            while(op == "+" || op == "-"){
                Ret nx1 = unit1(nx.ed + 1);
                if(!nx1.fl) return {false,0,0};
                if(op == "+") nx.val += nx1.val;
                if(op == "-") nx.val -= nx1.val;
                nx.ed = nx1.ed;
                op = code[nx1.ed].token;
            }
            return nx;
        }
        Ret unit3(int p){
            Ret nx = unit2(p);
            if(!nx.fl) return {false,0,0};
            string op = code[nx.ed].token;
            while(op == "<" || op == "<=" || op == ">" || op == ">="){
                Ret nx1 = unit2(nx.ed + 1);
                if(!nx1.fl) return {false,0,0};
                if(op == "<") nx.val = (nx.val < nx1.val);
                if(op == "<=") nx.val = (nx.val <= nx1.val);
                if(op == ">") nx.val = (nx.val > nx1.val);
                if(op == ">=") nx.val = (nx.val >= nx1.val);
                nx.ed = nx1.ed;
                op = code[nx1.ed].token;
            }
            return nx;
        }
        Ret unit4(int p){
            Ret nx = unit3(p);
            if(!nx.fl) return {false,0,0};
            string op = code[nx.ed].token;
            while(op == "==" || op == "!="){
                Ret nx1 = unit3(nx.ed + 1);
                if(!nx1.fl) return {false,0,0};
                if(op == "==") nx.val = (nx.val == nx1.val);
                if(op == "!=") nx.val = (nx.val != nx1.val);
                nx.ed = nx1.ed;
                op = code[nx1.ed].token;
            }
            return nx;
        }
        Ret unit5(int p){
            Ret nx = unit4(p);
            if(!nx.fl) return {false,0,0};
            string op = code[nx.ed].token;
            while(op == "^"){
                Ret nx1 = unit4(nx.ed + 1);
                if(!nx1.fl) return {false,0,0};
                if(op == "^") nx.val ^= nx1.val;
                nx.ed = nx1.ed;
                op = code[nx1.ed].token;
            }
            return nx;
        }
        Ret unit6(int p){
            Ret nx = unit5(p);
            if(!nx.fl) return {false,0,0};
            string op = code[nx.ed].token;
            while(op == "&&"){
                Ret nx1 = unit5(nx.ed + 1);
                if(!nx1.fl) return {false,0,0};
                if(op == "&&") nx.val = (nx.val && nx1.val);
                nx.ed = nx1.ed;
                op = code[nx1.ed].token;
            }
            return nx;
        }
        Ret unit7(int p){
            Ret nx = unit6(p);
            if(!nx.fl) return {false,0,0};
            string op = code[nx.ed].token;
            while(op == "||"){
                Ret nx1 = unit6(nx.ed + 1);
                if(!nx1.fl) return {false,0,0};
                if(op == "||") nx.val = (nx.val || nx1.val);
                nx.ed = nx1.ed;
                op = code[nx1.ed].token;
            }
            return nx;
        }
        //赋值
        Ret unit8(int p){
            Ret nx = unit7(p);if(!nx.fl) return {false,0,0};
            string op = code[nx.ed].token;
            if(op == "=" || op == "+=" || op == "-=" || op == "*=" || op == "/=" || op == "%=" || op == "<<=" || op == ">>=" || op == "^="){
                if(nx.ed - p != 1 || code[p].code != 0){
                    printf("lvalue required as left operand of assignment: %s\n",op.c_str());return {false,0,0};
                }
                string var = code[p].token;int scope_ptr = find_var(var);
                if(scope_ptr == -1){
                    printf("Variable '%s' was not declared in this scope.\n",var.c_str());return {false,0,0};
                }
                Ret nx1 = unit8(nx.ed + 1);
                if(!nx1.fl) return {false,0,0};
                //易错点：直接操作，而非先操作变量r。r作为返回值记得设置。
                if(op == "=") variables[scope_ptr][var] = nx1.val;
                else if(op == "+=") variables[scope_ptr][var] += nx1.val;
                else if(op == "-=") variables[scope_ptr][var] -= nx1.val;
                else if(op == "*=") variables[scope_ptr][var] *= nx1.val;
                else if(op == "/=") variables[scope_ptr][var] /= nx1.val;//暂不处理ZeroDivisionError
                else if(op == "%=") variables[scope_ptr][var] %= nx1.val;//暂不处理ZeroDivisionError
                else if(op == "<<=") variables[scope_ptr][var] <<= nx1.val;
                else if(op == ">>=") variables[scope_ptr][var] >>= nx1.val;
                else if(op == "^=") variables[scope_ptr][var] ^= nx1.val;
                nx.val = variables[scope_ptr][var];nx.ed = nx1.ed;
            }
            return nx;
        }
        //逗号运算符
        Ret unit9(int p){
            Ret nx = unit8(p);
            if(!nx.fl) return {false,0,0};
            while(code[nx.ed].token == ","){
                nx = unit8(nx.ed + 1);
                if(!nx.fl) return {false,0,0};
            }
            return nx;
        }
        //declaration只能初始化不能赋值。int a,b,c/*,m = 1*/;
        //变量声明当然是在最新作用域，即variables.back()
        Ret declaration(int p){
            string var = code[++p].token;
            EXPECT_VAR(code[p]);
            if(variables.back().count(var)){
                printf("Redeclaration of variable: '%s'.\n",var.c_str());
                return {false,0,0};
            }
            variables.back()[var] = 0;
            if(code[++p].token == ",") return declaration(p);
            return parse_semicolon(p);
        }
        Ret print(int p){
            ++p;
            if(code[p].token != "("){
                printf("Expected '(' but get token: %s\n",code[p].token.c_str());
                return {false,0,0};
            }
            vector<pair<string,int> > vars;
            while(1){
                ++p;
                EXPECT_VAR(code[p]);
                string var = code[p].token;int scope_ptr = find_var(var);
                if(scope_ptr == -1){
                    printf("Variable '%s' was not declared in this scope.\n",var.c_str());return {false,0,0};
                }
                vars.push_back(make_pair(var,variables[scope_ptr][var]));
                string symb = code[++p].token;
                if(symb != ")" && symb != ","){
                    puts("Invalid usage of function \"print\".");return {false,0,0};
                }
                if(symb == ")") break;
            }
            Ret nx = parse_semicolon(++p);if(!nx.fl) return {false,0,0};
            re_(i,0,vars.size()) printf("%s=%d%c",vars[i].first.c_str(),vars[i].second," \n"[i == vars.size() - 1]);//暂时采用的定义
            return {true,0,nx.ed};
        }
        Ret while_statement(int p){
            ++p;//skip "while"
            if(code[p].token != "("){
                printf("Expected '(' but get token: %s\n",code[p].token.c_str());return {false,0,0};
            }
            int tp = ++p;
            while(true){
                Ret nx = unit9(p);
                if(!nx.fl) return {false,0,0};
                p = nx.ed;
                if(code[p].token != ")"){
                    printf("Expected ')' but get token: %s\n",code[p].token.c_str());return {false,0,0};
                }
                int ed = ++p;//skip ")"
                if(code[p].token == "{"){
                    bool fl = match_bracket(p,ed);
                    if(!fl) return {false,0,0};
                }
                else{
                    for(;ed < code.size() && code[ed].token != ";";++ed);
                    if(ed >= code.size()){
                        puts("Expected ';' at while statement.");return {false,0,0};
                    }
                    ++ed;
                }
                if(!nx.val) return {true,0,ed};//指向while循环结束的地方
                if(code[p].token == "{"){
                    ++p;if(!statements(p)) return {false,0,0};
                }
                else{
                    Ret nx = statement(p);
                    if(!nx.fl) return {false,0,0};
                }
                p = tp;//指针移回原位
            }
            return {true,0,0};//不会运行到这
        }
        Ret statement(int p){
            string token = code[p].token;int cod = code[p].code;
            if(token == "print") return print(p);
            else if(token == "while") return while_statement(p);
            else if(cod == 0){//变量,故为赋值语句
                Ret nx = unit9(p);
                if(!nx.fl) return {false,0,0};
                return parse_semicolon(nx.ed);
            }
            //"int",故为声明语句
            else if(cod == 1) return declaration(p);
            puts("Invalid declaration statement, assignment statement or function call.");
            return {false,0,0};
        }
        //语句的集合，一定包含在"{}"内。注意statements()不跳过"}"
        bool statements(int p){
            variables.push_back(map<string,int>());//新建作用域
            Ret nx = statement(p);
            if(!nx.fl) return false;
            string symb = code[nx.ed].token;
            while(symb != "}"){
                nx = statement(nx.ed);
                if(!nx.fl) return false;
                symb = code[nx.ed].token;
            }
            variables.pop_back();//销毁作用域
            return true;
        }
        //已经确保返回类型都是int。暂时假设只有main一个函数
        bool run_function(int p){
            return statements(p + 5);
        }
        bool analyze(){
            if(!func_and_var()) return false;
            if(!func.count("main")){
                puts("Didn`t find function 'main'!");
                return false;
            }
            return run_function(func["main"]);
        }
    #undef EXPECT_VAR
};

int main(int argc, char** argv) {
    Lexer::init();
    while(1){
        int n;read(n);
        if(n <= 0) break;
        string s;
        rep(_,1,n){
            char s0[SZ];fgets(s0,SZ,stdin);
            s += s0;
        }
        Lexer lxr(s);
        bool legal = lxr.analyze();
        cout << "is lexical legal: " << (legal ? "true" : "false") << endl;
        if(!legal) continue;
        Parser psr(lxr.getRes());
        legal = psr.analyze();
        cout << "is syntax legal: " << (legal ? "true" : "false") << endl;
    }
    return 0;
}