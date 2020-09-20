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
        virtual ~Parser(){
            cout << "~Parser()" << endl;
        }
        
        Ret match_bracket(int st){
            int brkt = 1,ed = st + 1;
            if(code[st].token != "(" && code[st].token != "[" && code[st].token != "{"){
                printf("Expected bracket but get token: '%s'\n",code[st].token.c_str());
                return {false,0,0};
            }
            //特意安排了这些括号"(",")","[","]","{","}"的code相邻
            for(;ed < code.size() && brkt != 0;++ed){
                if(code[ed].code == code[st].code) brkt++;
                else if(code[ed].code == code[st].code + 1) brkt--;
            }
            if(brkt != 0){
                printf("Bracket '%s' doesn`t match.\n",code[st].token.c_str());return {false,0,0};
            }
            return {true,0,ed};
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
                Ret nx;
                if(code[p + 2].token != "("){
                    nx = declaration(p);
                    if(!nx.fl) return false;
                }
                else{
                    func[code[p + 1].token] = p;
                    nx = match_bracket(p + 2);
                    if(!nx.fl) return false;
                    int p1 = nx.ed;
                    if(code[p1].token != "{"){
                        puts("Function body should start with '{'!");return false;
                    }
                    nx = match_bracket(p1);
                    if(!nx.fl) return false;
                }
                p = nx.ed;
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
        Ret parse_semicolon(int p,int ret_val = 0){
            if(code[p].token != ";"){
                printf("Expected ';' but get token: '%s'\n",code[p].token.c_str());
                return {false,0,0};
            }
            ++p;return {true,ret_val,p};
        }
        //找到每种控制结构语句的结束位置。它顺便做了括号匹配的检查，调用者无需再检查。
        Ret next_statement(int p){
            if(code[p].token == "while" || code[p].token == "for"){
                Ret nx = match_bracket(p + 1);
                if(!nx.fl) return nx;
                return find_end_pos(nx.ed,code[p].token);
            }
            else if(code[p].token == "if"){
                Ret nx = match_bracket(p + 1);
                if(!nx.fl) return nx;
                nx = find_end_pos(nx.ed,code[p].token);
                if(!nx.fl) return nx;
                if(code[nx.ed].token != "else") return nx;
                return next_statement(nx.ed);
            }
            else if(code[p].token == "else"){
                ++p;
                if(code[p].token == "if") return next_statement(p);
                return find_end_pos(p,"else");
            }
            return find_end_pos(p,"");
        }
        
        Ret find_end_pos(int p,string name){
            if(code[p].token == "{" || code[p].token == "[" || code[p].token == "(") return match_bracket(p);
            if(code[p].token == "if" || code[p].token == "else" || code[p].token == "while" || code[p].token == "for" || code[p].token == "do") return next_statement(p);//若无括号且为控制结构语句，则递归调用next_statement找到正确结束位置
            for(;p < code.size() && code[p].token != ";";++p);
            if(p >= code.size()){//找下一个分号
                printf("Expected ';' at '%s' statement.\n",name.c_str());return {false,0,0};
            }
            return {true,0,++p};
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
            if(!nx.fl) return nx;
            string op = code[nx.ed].token;
            while(op == "*" || op == "/" || op == "%"){
                Ret nx1 = unit0(nx.ed + 1);
                if(!nx1.fl) return nx1;
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
            if(!nx.fl) return nx;
            string op = code[nx.ed].token;
            while(op == "+" || op == "-"){
                Ret nx1 = unit1(nx.ed + 1);
                if(!nx1.fl) return nx1;
                if(op == "+") nx.val += nx1.val;
                if(op == "-") nx.val -= nx1.val;
                nx.ed = nx1.ed;
                op = code[nx1.ed].token;
            }
            return nx;
        }
        Ret unit3(int p){
            Ret nx = unit2(p);
            if(!nx.fl) return nx;
            string op = code[nx.ed].token;
            while(op == "<" || op == "<=" || op == ">" || op == ">="){
                Ret nx1 = unit2(nx.ed + 1);
                if(!nx1.fl) return nx1;
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
            if(!nx.fl) return nx;
            string op = code[nx.ed].token;
            while(op == "==" || op == "!="){
                Ret nx1 = unit3(nx.ed + 1);
                if(!nx1.fl) return nx1;
                if(op == "==") nx.val = (nx.val == nx1.val);
                if(op == "!=") nx.val = (nx.val != nx1.val);
                nx.ed = nx1.ed;
                op = code[nx1.ed].token;
            }
            return nx;
        }
        Ret unit5(int p){
            Ret nx = unit4(p);
            if(!nx.fl) return nx;
            string op = code[nx.ed].token;
            while(op == "^"){
                Ret nx1 = unit4(nx.ed + 1);
                if(!nx1.fl) return nx1;
                if(op == "^") nx.val ^= nx1.val;
                nx.ed = nx1.ed;
                op = code[nx1.ed].token;
            }
            return nx;
        }
        Ret unit6(int p){
            Ret nx = unit5(p);
            if(!nx.fl) return nx;
            string op = code[nx.ed].token;
            while(op == "&&"){
                Ret nx1 = unit5(nx.ed + 1);
                if(!nx1.fl) return nx1;
                if(op == "&&") nx.val = (nx.val && nx1.val);
                nx.ed = nx1.ed;
                op = code[nx1.ed].token;
            }
            return nx;
        }
        Ret unit7(int p){
            Ret nx = unit6(p);
            if(!nx.fl) return nx;
            string op = code[nx.ed].token;
            while(op == "||"){
                Ret nx1 = unit6(nx.ed + 1);
                if(!nx1.fl) return nx1;
                if(op == "||") nx.val = (nx.val || nx1.val);
                nx.ed = nx1.ed;
                op = code[nx1.ed].token;
            }
            return nx;
        }
        //赋值
        Ret unit8(int p){
            Ret nx = unit7(p);if(!nx.fl) return nx;
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
                if(!nx1.fl) return nx1;
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
            if(!nx.fl) return nx;
            while(code[nx.ed].token == ","){
                nx = unit8(nx.ed + 1);
                if(!nx.fl) return nx;
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
        //所有循环暂不支持条件部分为空，因为还没有实现break;
        Ret while_statement(int p){
            Ret ed_obj = next_statement(p);
            if(!ed_obj.fl) return ed_obj;
            int tp = p + 2;
            while(true){
                if(code[tp].token == ")"){
                    puts("Because we don`t have 'break', we don`t support null condition in whileloop as well.");
                    return {false,0,0};
                }
                Ret nx = unit9(tp);if(!nx.fl) return nx;
                if(!nx.val) return ed_obj;//满足退出条件，指向while结束的地方
                //nx.ed + 1可能是"{"或其他
                if(!statements(nx.ed + 1,ed_obj.ed)) return {false,0,0};
            }
            return {true,-1,-1};//不会运行到这
        }
        Ret for_statement(int p){
            Ret ed_obj = next_statement(p);
            if(!ed_obj.fl) return ed_obj;
            int tp = p + 2;
            Ret rig_brkt = match_bracket(p + 1);//next_statement()已保证括号匹配
            variables.push_back(map<string,int>());//新建作用域
            if(code[tp].code == 0){
                Ret nx = expres_statement(tp);
                if(!nx.fl) return nx;
                tp = nx.ed;
            }
            else if(code[tp].code == 1){
                Ret nx = declaration(tp);
                if(!nx.fl) return nx;
                tp = nx.ed;
            }
            else if(code[tp].token == ";") ++tp;//空语句
            else{
                puts("Illegal init statement at forloop.");
                return {false,0,0};
            }
            vector<string> preserve_list;
            for(auto x: variables.back()) preserve_list.push_back(x.first);
            while(true){
                if(code[tp].token == ";"){
                    puts("Because we don`t have 'break', we don`t support null condition in forloop as well.");
                    return {false,0,0};
                }
                //tp、nx.ed分别是1st、2nd ";"的下一个位置
                Ret nx = expres_statement(tp);if(!nx.fl) return nx;
                if(!nx.val){
                    variables.pop_back();//销毁作用域
                    return ed_obj;//满足退出条件，指向for结束的地方
                }
                //rig_brkt.ed可能是"{"或其他
                if(!statements(rig_brkt.ed,ed_obj.ed,0)) return {false,0,0};
                if(code[nx.ed].token != ")"){//非空语句
                    nx = unit9(nx.ed);if(!nx.fl) return nx;
                }
                //保留在init部分声明的变量,其余删除
                map<string,int> preserve_vars;
                for(auto plst: preserve_list) preserve_vars[plst] = variables.back()[plst];
                variables[variables.size() - 1] = preserve_vars;
            }
            return {true,-1,-1};//不会运行到这
        }
        Ret if_statement(int p){
            Ret ed_obj = next_statement(p);
            if(!ed_obj.fl) return ed_obj;
            int tp = p + 2;
            Ret nx = unit9(tp);if(!nx.fl) return nx;
            p = nx.ed + 1;
            Ret else_obj = find_end_pos(p,"if");
            int else_pos = else_obj.ed;
            if(!nx.val){
                if(code[else_pos].token == "else"){
                    ++else_pos;
                    if(code[else_pos].token == "if") return statement(else_pos);
                    //若else后无if
                    if(!statements(else_pos,find_end_pos(else_pos,"else").ed)) return {false,0,0};
                }
                return ed_obj;
            }
            if(!statements(p,else_pos)) return {false,0,0};
            return ed_obj;
        }
        Ret expres_statement(int p){//(非空的)赋值语句
            Ret nx = unit9(p);
            if(!nx.fl) return nx;
            return parse_semicolon(nx.ed,nx.val);
        }
        Ret statement(int p){
            string token = code[p].token;int cod = code[p].code;
            if(token == "print") return print(p);
            else if(token == "while") return while_statement(p);
            else if(token == "for") return for_statement(p);
            else if(token == "if") return if_statement(p);
            else if(token == ";") return {true,0,++p};//空语句
            else if(cod == 0) return expres_statement(p);
            else if(cod == 1) return declaration(p);//"int",故为声明语句
            if(token == "else") puts("Unmatched 'else'.");
            else puts("We can`t support such statement currently.");
            return {false,0,0};
        }
        //lim：结束位置,op：新建作用域的数量
        bool statements(int p,int lim,int op = 1){
            if(op) variables.push_back(map<string,int>());//新建作用域
            if(code[p].token != "{"){
                Ret nx = statement(p);
                if(!nx.fl) return false;
            }
            else if(code[p + 1].token != "}"){//非空body
                Ret nx = statement(++p);
                if(!nx.fl) return false;
                while(nx.ed < lim - 1){
                    nx = statement(nx.ed);
                    if(!nx.fl) return false;
                }
            }
            if(op) variables.pop_back();//销毁作用域
            return true;
        }
        //已经确保返回类型都是int。暂时假设只有main一个函数
        bool run_function(int p){
            return statements(p + 4,match_bracket(p + 4).ed);
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