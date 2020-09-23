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
不合法时返回：Ret(false);*/
const int CMD_RETURN = 1,CMD_BREAK = 2,CMD_CONTINUE = 3;
struct Ret{
    bool fl;int val;//返回值
    int ed;//终止位置(为了避免使用全局下标，同时解决通信问题)
    int cmd;//命令:如return
    Ret(){}
    Ret(bool fl):fl(fl){val = ed = cmd = 0;}
    Ret(bool fl,int val,int ed,int cmd):fl(fl),val(val),ed(ed),cmd(cmd){}
};

//仅变量自己知道是否是for循环初始化区的变量。
const int FOR_INIT = 1;
struct Var{
    int v,for_init;
    Var(){}
    Var(int v,int for_init):v(v),for_init(for_init){}
};
//作用域类型(默认建立全局作用域)
const int SCP_GLOBAL = -1,SCP_FUNC = 1,SCP_WHILE = 2,SCP_FOR = 3,SCP_IF = 4,SCP_DOWHILE = 5;
struct Scope{
    map<string,Var> variables;int typ;
    Scope(){typ = SCP_GLOBAL;}
    Scope(int typ):typ(typ){}
    bool new_var(string name,int val,int for_init = 0){
        if(variables.count(name)){
            printf("Redeclaration of variable: '%s'.\n",name.c_str());
            return false;
        }
        variables[name] = Var(val,for_init);//新定义变量获得默认值0
        return true;
    }
    //保留在for_init部分声明的变量,其余删除
    void del_vars_forloop(){
        if(typ != SCP_FOR) return;
        for(auto it = variables.begin();it != variables.end();){
            if(it->second.for_init == FOR_INIT) it++;
            else variables.erase(it++);
        }
    }
};
//作用域自动管理器：for循环等不必自行管理作用域
struct ScopeManager{
    vector<Scope> scopes;
    void new_scope(int scp_typ){
        scopes.push_back(Scope(scp_typ));
    }
    bool new_var(string name,int val,int for_init = 0){
        return scopes.back().new_var(name,val,for_init);
    }
    //在父作用域寻找变量,返回：v属性:变量值,ed属性:下标
    Ret find_var(string var){
        dwn(i,scopes.size() - 1,1){
            if(scopes[i].variables.count(var)) return Ret(true,scopes[i].variables[var].v,i,0);
            if(scopes[i].typ == SCP_FUNC) break;
        }
        if(scopes[0].variables.count(var)) return Ret(true,scopes[0].variables[var].v,0,0);
        printf("Variable '%s' was not declared in this scope.\n",var.c_str());
        return Ret(false);
    }
    Ret mdy_var(string var,int v){
        Ret found = find_var(var);
        if(!found.fl) return found;
        scopes[found.ed].variables[var].v = v;
        return Ret(true,v,found.ed,0);
    }
    Ret mdy_var(string var,int v,string op){//用于赋值语句
        Ret found = find_var(var);
        if(!found.fl) return found;
        if(op == "=") scopes[found.ed].variables[var].v = v;
        else if(op == "+=") scopes[found.ed].variables[var].v += v;
        else if(op == "-=") scopes[found.ed].variables[var].v -= v;
        else if(op == "*=") scopes[found.ed].variables[var].v *= v;
        else if(op == "/=") scopes[found.ed].variables[var].v /= v;//暂不处理ZeroDivisionError
        else if(op == "%=") scopes[found.ed].variables[var].v %= v;//暂不处理ZeroDivisionError
        else if(op == "<<=") scopes[found.ed].variables[var].v <<= v;
        else if(op == ">>=") scopes[found.ed].variables[var].v >>= v;
        else if(op == "&=") scopes[found.ed].variables[var].v &= v;
        else if(op == "^=") scopes[found.ed].variables[var].v ^= v;
        else if(op == "|=") scopes[found.ed].variables[var].v |= v;
        return Ret(true,scopes[found.ed].variables[var].v,found.ed,0);
    }
    void del_scope(){
        if(scopes.back().typ == SCP_FOR)
            scopes.back().del_vars_forloop();
        else scopes.pop_back();
    }
    //仅for循环使用
    void truly_del_scope(){
        scopes.pop_back();
    }
    bool has_loop_scope(string loop_name){
        dwn(i,scopes.size() - 1,1){
            if(scopes[i].typ == SCP_WHILE || scopes[i].typ == SCP_FOR || scopes[i].typ == SCP_DOWHILE) return true;
            if(scopes[i].typ == SCP_FUNC) break;
        }
        printf("Illegal '%s' statement.\n",loop_name.c_str());
        return false;
    }
};

struct Func{
    int left_bkt;vector<string> param_names;
    Func(){}
    Func(int left_bkt,vector<string> param_names):left_bkt(left_bkt),param_names(param_names){}
};
struct FuncManager{
    map<string,Func> funs;
    string print(string name){
        string s = "int " + name + "(";
        int sz = funs[name].param_names.size();
        if(!sz) s += ")";
        for(int i = 0;i < sz;++i){
            s += "int";
            s += i < sz - 1 ? "," : ")";
        }
        return s;
    }
    int get_statement_pos(string fname){
        return funs[fname].left_bkt;
    }
    int param_num(string fname){
        return funs[fname].param_names.size();
    }
    vector<string> get_params(string fname){
        return funs[fname].param_names;
    }
};

class Parser{
    //已读取的情况下，期望是变量
    #define EXPECT_VAR(u) if((u).code != 0){printf("Expected variable but get token: %s\n",(u).token.c_str());return Ret(false);}
    private:
        vector<Node> code;
        FuncManager f_mgr;
        ScopeManager spmgr;
    public:
        Parser(){spmgr.new_scope(SCP_GLOBAL);}
        Parser(vector<Node> lex_unit):code(lex_unit){spmgr.new_scope(SCP_GLOBAL);}
        virtual ~Parser(){
            cout << "~Parser()" << endl;
        }
        
        Ret match_bracket(int st){
            int brkt = 1,ed = st + 1;
            if(code[st].token != "(" && code[st].token != "[" && code[st].token != "{"){
                printf("Expected bracket but get token: '%s'\n",code[st].token.c_str());
                return Ret(false);
            }
            //特意安排了这些括号"(",")","[","]","{","}"的code相邻
            for(;ed < code.size() && brkt != 0;++ed){
                if(code[ed].code == code[st].code) brkt++;
                else if(code[ed].code == code[st].code + 1) brkt--;
            }
            if(brkt != 0){
                printf("Bracket '%s' doesn`t match.\n",code[st].token.c_str());return Ret(false);
            }
            return Ret(true,0,ed,0);
        }
        
        //p是分号的位置
        Ret parse_semicolon(int p,int ret_val = 0){
            if(code[p].token != ";"){
                printf("Expected ';' but get token: '%s'\n",code[p].token.c_str());
                return Ret(false);
            }
            ++p;return Ret(true,ret_val,p,0);
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
                printf("Expected ';' at '%s' statement.\n",name.c_str());return Ret(false);
            }
            return Ret(true,0,++p,0);
        }
        //暂不处理函数重名的问题
        bool parse_func_defi(int p,int lim,string fun_name){
            vector<string> names;
            p += 3;
            if(code[p].token == ")"){
                f_mgr.funs[fun_name] = Func(lim,names);
                return true;
            }
            while(p < lim){
                if(code[p].code != 1){
                    printf("Unsupported param type '%s' of function '%s'\n",code[p].token.c_str(),fun_name);return false;
                }
                ++p;
                if(code[p].code != 0){
                    printf("Illegal param name '%s' of function '%s'\n",code[p].token.c_str(),fun_name);return false;
                }
                names.push_back(code[p].token);
                ++p;
                if(code[p].token != "," && code[p].token != ")"){
                    printf("Unexpected token '%s' in definition of function '%s'\n",code[p].token.c_str(),fun_name);return false;
                }
                ++p;
            }
            f_mgr.funs[fun_name] = Func(lim,names);
            return true;
        }
        //全局范围，要么是函数要么是声明语句
        bool func_and_var(){
            int p = 0;
            while(p < code.size()){
                if(code[p].code != 1){
                    printf("Unsupported return type at global scope: '%s'\n",code[p].token.c_str());return false;
                }
                if(code[p + 1].code != 0){
                    printf("Illegal function name or variable name: '%s'\n",code[p + 1].token.c_str());return false;
                }
                if(code[p + 2].token != "("){
                    Ret nx = declaration(p);
                    if(!nx.fl) return false;
                    p = nx.ed;
                }
                //parse函数定义
                else{
                    string fun_name = code[p + 1].token;
                    Ret nx = match_bracket(p + 2);
                    if(!nx.fl) return false;
                    int p1 = nx.ed;
                    if(code[p1].token != "{"){
                        puts("Function body should start with '{'!");return false;
                    }
                    Ret ed_obj = match_bracket(p1);
                    if(!ed_obj.fl) return false;
                    bool fl = parse_func_defi(p,p1,fun_name);
                    if(!fl) return false;
                    p = ed_obj.ed;
                }
            }
            return true;
        }
        //处理函数调用
        Ret parse_func_call(int p,string var){
            vector<int> vals;++p;
            if(code[p].token == ")") return run_function(var,vals);
            while(true){
                Ret nx = unit10(p);
                if(!nx.fl) return Ret(false);
                p = nx.ed;
                if(code[p].token != "," && code[p].token != ")"){
                    printf("Invalid function call to function '%s'.\n",f_mgr.print(var).c_str());
                    return Ret(false);
                }
                vals.push_back(nx.val);
                if(code[p].token == ")") break;
                ++p;
            }
            return run_function(var,vals);
        }
        //变量和函数重名时,优先认为它是函数。
        Ret unit0(int p){
            Node u = code[p];++p;
            if(!u.code){
                string var = u.token;
                if(f_mgr.funs.count(var)){
                    if(code[p].token != "("){
                        puts("Function call should use token '('.");
                        return Ret(false);
                    }
                    Ret ed_obj = match_bracket(p);
                    if(!ed_obj.fl) return ed_obj;
                    Ret fobj = parse_func_call(p,var);
                    return Ret(fobj.fl,fobj.val,ed_obj.ed,0);
                }
                Ret found = spmgr.find_var(var);
                if(!found.fl) return found;
                return Ret(true,found.val,p,0);
            }
            else if(u.code == INTCODE) return Ret(true,to_int(u.token),p,0);
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
                Ret nx = unit11(p);
                if(!nx.fl) return Ret(false);
                if(code[nx.ed].token != ")"){
                    puts("Syntax error! Wanna ')' token!");return Ret(false);
                }
                ++nx.ed;return nx;
            }
            printf("Unknown token: %s\n",u.token.c_str());
            return Ret(false);
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
            while(op == "&"){
                Ret nx1 = unit4(nx.ed + 1);
                if(!nx1.fl) return nx1;
                if(op == "&") nx.val &= nx1.val;
                nx.ed = nx1.ed;
                op = code[nx1.ed].token;
            }
            return nx;
        }
        Ret unit6(int p){
            Ret nx = unit5(p);
            if(!nx.fl) return nx;
            string op = code[nx.ed].token;
            while(op == "^"){
                Ret nx1 = unit5(nx.ed + 1);
                if(!nx1.fl) return nx1;
                if(op == "^") nx.val ^= nx1.val;
                nx.ed = nx1.ed;
                op = code[nx1.ed].token;
            }
            return nx;
        }
        Ret unit7(int p){
            Ret nx = unit6(p);
            if(!nx.fl) return nx;
            string op = code[nx.ed].token;
            while(op == "|"){
                Ret nx1 = unit6(nx.ed + 1);
                if(!nx1.fl) return nx1;
                if(op == "|") nx.val |= nx1.val;
                nx.ed = nx1.ed;
                op = code[nx1.ed].token;
            }
            return nx;
        }
        Ret unit8(int p){
            Ret nx = unit7(p);
            if(!nx.fl) return nx;
            string op = code[nx.ed].token;
            while(op == "&&"){
                Ret nx1 = unit7(nx.ed + 1);
                if(!nx1.fl) return nx1;
                if(op == "&&") nx.val = (nx.val && nx1.val);
                nx.ed = nx1.ed;
                op = code[nx1.ed].token;
            }
            return nx;
        }
        Ret unit9(int p){
            Ret nx = unit8(p);
            if(!nx.fl) return nx;
            string op = code[nx.ed].token;
            while(op == "||"){
                Ret nx1 = unit8(nx.ed + 1);
                if(!nx1.fl) return nx1;
                if(op == "||") nx.val = (nx.val || nx1.val);
                nx.ed = nx1.ed;
                op = code[nx1.ed].token;
            }
            return nx;
        }
        //赋值
        Ret unit10(int p){
            Ret nx = unit9(p);if(!nx.fl) return nx;
            string op = code[nx.ed].token;
            if(op == "=" || op == "+=" || op == "-=" || op == "*=" || op == "/=" || op == "%=" || op == "<<=" || op == ">>=" || op == "&=" || op == "^=" || op == "|="){
                if(nx.ed - p != 1 || code[p].code != 0){
                    printf("lvalue required as left operand of assignment: %s\n",op.c_str());return Ret(false);
                }
                string var = code[p].token;
                Ret found = spmgr.find_var(var);
                if(!found.fl) return found;
                Ret nx1 = unit10(nx.ed + 1);
                if(!nx1.fl) return nx1;
                //易错点：直接操作，而非先操作变量nx.val。nx.val作为返回值记得设置。
                Ret mdy = spmgr.mdy_var(var,nx1.val,op);
                nx.val = mdy.val;nx.ed = nx1.ed;
            }
            return nx;
        }
        //逗号运算符
        Ret unit11(int p){
            Ret nx = unit10(p);
            if(!nx.fl) return nx;
            while(code[nx.ed].token == ","){
                nx = unit10(nx.ed + 1);
                if(!nx.fl) return nx;
            }
            return nx;
        }
        //declaration只能初始化不能赋值。int a,b,c/*,m = 1*/;
        //变量声明当然是在最新作用域(已封装)
        Ret declaration(int p,int for_init = 0){
            string var = code[++p].token;
            /* 暂时留着,因为莫名其妙出现“Expected variable but get token: ”
            cout <<"dec "<<code[p].token<<" "<<code[p].code<<endl;//
            */
            EXPECT_VAR(code[p]);++p;
            int val = 0;
            if(code[p].token == "="){
                Ret nx = unit10(p + 1);
                if(!nx.fl) return nx;
                val = nx.val;
                p = nx.ed;
            }
            if(!spmgr.new_var(var,val,for_init)) return Ret(false);
            if(code[p].token == ",") return declaration(p,for_init);//补丁：别忘了递归的时候传值...否则for循环只有1个变量被认为是不应该删
            return parse_semicolon(p);
        }
        Ret print(int p){
            ++p;
            if(code[p].token != "("){
                printf("Expected '(' but get token: %s\n",code[p].token.c_str());
                return Ret(false);
            }
            vector<pair<string,int> > vars;
            while(1){
                ++p;
                EXPECT_VAR(code[p]);
                string var = code[p].token;
                Ret found = spmgr.find_var(var);
                if(!found.fl) return found;
                vars.push_back(make_pair(var,found.val));
                string symb = code[++p].token;
                if(symb != ")" && symb != ","){
                    puts("Invalid usage of function \"print\".");return Ret(false);
                }
                if(symb == ")") break;
            }
            Ret nx = parse_semicolon(++p);if(!nx.fl) return Ret(false);
            re_(i,0,vars.size()) printf("%s=%d%c",vars[i].first.c_str(),vars[i].second," \n"[i == vars.size() - 1]);//暂时采用的定义
            return Ret(true,0,nx.ed,0);
        }
        //所有循环暂不支持条件部分为空，因为还没有实现break;
        Ret while_statement(int p){
            Ret ed_obj = next_statement(p);
            if(!ed_obj.fl) return ed_obj;
            int tp = p + 2;
            for(Ret stmts(true);true;){
                if(code[tp].token == ")"){
                    puts("Even if we have 'break', we don`t support null condition in whileloop as well.");
                    return Ret(false);
                }
                //补丁：退出循环之前不应该重新计算判定条件！
                if(stmts.cmd == CMD_RETURN || stmts.cmd == CMD_BREAK){
                    if(stmts.cmd == CMD_BREAK) return Ret(true,0,ed_obj.ed,0);
                    return Ret(true,stmts.val,ed_obj.ed,stmts.cmd);
                }
                Ret nx = unit11(tp);if(!nx.fl) return nx;
                //满足退出条件，指向while结束的地方
                if(!nx.val) return Ret(true,0,ed_obj.ed,0);
                stmts = statements(nx.ed + 1,ed_obj.ed,SCP_WHILE);//nx.ed + 1是"{"或其他
                if(!stmts.fl) return Ret(false);
            }
            return {true,-1,-1,-1};//不会运行到这
        }
        Ret for_statement(int p){
            Ret ed_obj = next_statement(p);
            if(!ed_obj.fl) return ed_obj;
            int tp = p + 2;
            Ret rig_brkt = match_bracket(p + 1);//next_statement()已保证括号匹配
            spmgr.new_scope(SCP_FOR);//新建作用域
            if(code[tp].code == 0 || code[tp].token == ";"){
                Ret nx = expres_statement(tp);//赋值语句或空语句
                if(!nx.fl) return nx;
                tp = nx.ed;
            }
            else if(code[tp].code == 1){
                Ret nx = declaration(tp,FOR_INIT);
                if(!nx.fl) return nx;
                tp = nx.ed;
            }
            else{
                puts("Illegal init statement at forloop.");return Ret(false);
            }
            for(Ret stmts(true);true;){
                if(code[tp].token == ";"){
                    puts("Even if we have 'break', we don`t support null condition in forloop as well.");return Ret(false);
                }
                //补丁：退出循环之前不应该重新计算判定条件！
                if(stmts.cmd == CMD_RETURN || stmts.cmd == CMD_BREAK){
                    spmgr.truly_del_scope();//真实的销毁作用域
                    if(stmts.cmd == CMD_BREAK) return Ret(true,0,ed_obj.ed,0);
                    return Ret(true,stmts.val,ed_obj.ed,stmts.cmd);
                }
                //tp、nx.ed分别是1st、2nd ";"的下一个位置
                Ret nx = expres_statement(tp);if(!nx.fl) return nx;
                //满足退出条件，指向for结束的地方
                if(!nx.val){
                    spmgr.truly_del_scope();
                    return Ret(true,0,ed_obj.ed,0);
                }
                stmts = statements(rig_brkt.ed,ed_obj.ed,SCP_FOR,0);//rig_brkt.ed是"{"或其他
                if(!stmts.fl) return Ret(false);
                if(code[nx.ed].token != ")"){//非空语句
                    if(code[nx.ed].code != 0){
                        puts("Forloop`s 3rd statement should be an expression.");return Ret(false);
                    }
                    nx = unit11(nx.ed);if(!nx.fl) return nx;
                }
                spmgr.del_scope();//清空循环体定义的变量
            }
            return Ret(true,-1,-1,-1);//不会运行到这
        }
        Ret if_statement(int p){
            Ret ed_obj = next_statement(p);
            if(!ed_obj.fl) return ed_obj;
            int tp = p + 2;
            Ret nx = unit11(tp);if(!nx.fl) return nx;
            p = nx.ed + 1;
            Ret else_obj = find_end_pos(p,"if");
            int else_pos = else_obj.ed;
            Ret stmts(true);
            if(!nx.val){
                if(code[else_pos].token == "else"){
                    ++else_pos;
                    if(code[else_pos].token == "if") return statement(else_pos);
                    //若else后无if
                    stmts = statements(else_pos,find_end_pos(else_pos,"else").ed,SCP_IF);
                    if(!stmts.fl) return stmts;
                }
            }
            else{
                stmts = statements(p,else_pos,SCP_IF);
                if(!stmts.fl) return stmts;
            }
            return Ret(true,stmts.val,ed_obj.ed,stmts.cmd);
        }
        Ret expres_statement(int p){//赋值语句
            if(code[p].token == ";") return Ret(true,0,++p,0);//空语句(return;默认为return 0;)
            Ret nx = unit11(p);
            if(!nx.fl) return nx;
            return parse_semicolon(nx.ed,nx.val);
        }
        //处理语句(块)
        Ret statement(int p){
            string token = code[p].token;int cod = code[p].code;
            if(token == "print") return print(p);
            else if(token == "while") return while_statement(p);
            else if(token == "for") return for_statement(p);
            else if(token == "if") return if_statement(p);
            else if(token == ";") return Ret(true,0,++p,0);//空语句
            else if(token == "return"){
                Ret nx = expres_statement(++p);
                if(!nx.fl) return nx;
                return Ret(true,nx.val,0,CMD_RETURN);
            }
            else if(token == "break"){
                if(!spmgr.has_loop_scope("break")) return Ret(false);
                if(!parse_semicolon(p + 1).fl) return Ret(false);
                return Ret(true,0,0,CMD_BREAK);
            }
            else if(token == "continue"){
                if(!spmgr.has_loop_scope("continue")) return Ret(false);
                if(!parse_semicolon(p + 1).fl) return Ret(false);
                return Ret(true,0,0,CMD_CONTINUE);
            }
            else if(cod == 0) return expres_statement(p);
            else if(cod == 1) return declaration(p);//"int",故为声明语句
            if(token == "else") puts("Unmatched 'else'.");
            else puts("We can`t support such statement currently.");
            return Ret(false);
        }
        //lim：结束位置,op：新建作用域的数量(仅for循环使用)
        //scp_typ：statements应该知道这段语句属于哪个块
        Ret statements(int p,int lim,int scp_typ,int op = 1){
            if(op) spmgr.new_scope(scp_typ);//新建作用域
            Ret nx;
            if(code[p].token != "{"){
                nx = statement(p);
                if(!nx.fl) return nx;
            }
            else if(code[p + 1].token != "}"){//非空body
                for(++p;p < lim - 1;p = nx.ed){
                    nx = statement(p);
                    if(!nx.fl) return nx;
                    if(nx.cmd == CMD_RETURN || nx.cmd == CMD_BREAK || nx.cmd == CMD_CONTINUE){
                        if(op) spmgr.del_scope();//销毁作用域
                        return nx;
                    }
                }
            }
            if(op) spmgr.del_scope();//销毁作用域
            return nx;
        }
        //已经确保返回类型都是int。暂时假设只有main一个函数
        Ret run_function(string name,vector<int> params){
            int sz1 = params.size(),sz2 = f_mgr.param_num(name);
            if(sz1 != sz2){
                printf("Too %s arguments to function '%s'\n",sz1 < sz2 ? "few" : "many",f_mgr.print(name).c_str());
                return Ret(false);
            }
            spmgr.new_scope(SCP_FUNC);
            vector<string> vars = f_mgr.get_params(name);
            for(int i = 0;i < sz1;++i){
                spmgr.new_var(vars[i],params[i]);
            }
            int pos = f_mgr.get_statement_pos(name);
            Ret ret = statements(pos,match_bracket(pos).ed,SCP_FUNC,0);
            if(!ret.fl) return ret;
            spmgr.del_scope();
            return ret;
        }
        bool analyze(){
            if(!func_and_var()) return false;
            if(!f_mgr.funs.count("main")){
                puts("Didn`t find function 'main'!");
                return false;
            }
            return run_function("main",vector<int>()).fl;
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