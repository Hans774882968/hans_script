#include <bits/stdc++.h>
using namespace std;
typedef long long LL;
#define rep(i,a,b) for(int i = (a);i <= (b);++i)
#define re_(i,a,b) for(int i = (a);i < (b);++i)
#define dwn(i,a,b) for(int i = (a);i >= (b);--i)

//utils.cpp
//regex_replace不能正确匹配若干反斜杠这种字符串
string myreplace(string s,string old,string cur){
    int pos = 0;string ret;
    while(pos < s.size()){
        int pos1 = s.find(old,pos);
        if(pos1 == string::npos){ret += s.substr(pos);break;}
        else ret += s.substr(pos,pos1 - pos) + cur,pos = pos1 + old.size();
    }
    return ret;
}

string clean_str(string s){
    s = myreplace(s,"\\\\","\\");
    s = myreplace(s,"\\\"","\"");
    return s;
}

bool judge_binary(string d){
    re_(i,0,d.size())
        if(isalpha(d[i]) || d[i] - '0' >= 2)
            return false;
    return true;
}
bool judge_octal(string d){
    re_(i,0,d.size())
        if(isalpha(d[i]) || d[i] - '0' >= 8)
            return false;
    return true;
}
bool judge_hex(string d){
    re_(i,0,d.size())
        if(isalpha(d[i]) && tolower(d[i]) - 'a' >= 6)
            return false;
    return true;
}
bool judge_integer(string d){
    if(d[0] == '0'){
        if(isalpha(d[1])){
            if(tolower(d[1]) == 'x'){
                bool legal = judge_hex(d.substr(2));
                if(!legal){
                    printf("Illegal hex number: %s\n",d.c_str());
                    return false;
                }
            }
            else if(tolower(d[1]) == 'b'){
                bool legal = judge_binary(d.substr(2));
                if(!legal){
                    printf("Illegal binary number: %s\n",d.c_str());
                    return false;
                }
            }
            else{
                printf("Illegal number: %s\n",d.c_str());
                return false;
            }
        }
        else{
            bool legal = judge_octal(d.substr(1));
            if(!legal){
                printf("Illegal octal number: %s\n",d.c_str());
                return false;
            }
        }
    }
    return true;
}

//'\n'长度4,'a'长度3
bool judge_char(string s){
    if(s[1] == '\\') return s.size() == 4;
    return s.size() == 3;
}

//trie.cpp
struct Trie{
    static const int LEN = 70,ALPHA = 260;
    int T[LEN][ALPHA],end[LEN],sz,wd_num;
    map<char,int> mp;int msz;
    Trie(){
        msz = sz = wd_num = 0;
        memset(T[0],0,sizeof T[0]);
        end[0] = -1;
    }
    
    inline int ID(char x){
        if(!mp.count(x)) return mp[x] = ++msz;
        return mp[x];
    }
    void insert(string s){
        int u = 0;
        for(auto x: s){
            int v = ID(x);
            if(!T[u][v]){
                memset(T[++sz],0,sizeof T[sz]);
                end[sz] = -1;
                T[u][v] = sz;
            }
            u = T[u][v];
        }
        if(end[u] == -1) end[u] = wd_num++;
    }
    int in_dict(string s){
        int u = 0;
        for(auto x: s){
            int v = ID(x);
            if(!T[u][v]) return -1;
            u = T[u][v];
        }
        return end[u];
    }
    int get_longest_prefix(string s){
        int u = 0,len = 0;
        for(auto x: s){
            int v = ID(x);
            if(!T[u][v]) return len;
            u = T[u][v];
            ++len;
        }
        return len;
    }
};

//lexer.cpp
const int SZ = 10000 + 5;
const int KWDNUM = 11,SYMNUM = 44,QUONUM = 2;
const int INTCODE = KWDNUM + SYMNUM + QUONUM + 1;

struct Node{
    string token;int code;
    Node(){}
    Node(string token,int code):token(token),code(code){}
};

class Lexer{
    private:
        string s;int idx;
        vector<Node> res;
        static const string kwd[KWDNUM];
        static const string symbol[SYMNUM];
        static const string quot[QUONUM];
        static set<char> symbol_ch;
        static Trie symbol_dict,kwd_dict;
    public:
        Lexer(){idx = 0;}
        Lexer(string s):s(s){idx = 0;}
        
        vector<Node> getRes(){return res;}
        static void init(){
            re_(i,0,SYMNUM){
                for(auto x: symbol[i]) symbol_ch.insert(x);
                symbol_dict.insert(symbol[i]);
            }
            re_(i,0,KWDNUM){
                kwd_dict.insert(kwd[i]);
            }
        }
        
        int is_symbol_str(string symb){
            return symbol_dict.in_dict(symb);
        }
        int is_keyword_str(string token){
            return kwd_dict.in_dict(token);
        }
        int is_quote(string token){
            re_(i,0,QUONUM) if(quot[i][0] == token[0]) return i;
            return -1;
        }
        int get_token_id(string token){
            int idx = is_keyword_str(token);
            if(~idx) return idx + 1;
            idx = is_symbol_str(token);
            if(~idx) return idx + KWDNUM + 1;
            idx = is_quote(token);
            if(~idx) return idx + KWDNUM + SYMNUM + 1;
            return 0;
        }
        const char* get_token_type_name(int code){
            if(!code) return "variable";
            if(code <= KWDNUM) return "keyword";
            if(code <= KWDNUM + SYMNUM) return "symbol";
            if(code == KWDNUM + SYMNUM + 1) return "const_character";
            if(code == KWDNUM + SYMNUM + 2) return "const_string";
            return "integer";
        }
        void print_result(){
            for(auto rs: res) printf("(%s,%s,%d)\n",rs.token.c_str(),get_token_type_name(rs.code),rs.code);
        }
        bool is_symbol(char ch){
            return symbol_ch.find(ch) != symbol_ch.end();
        }
        
        string get_identifier(){
            int tidx = idx;
            for(;idx < s.size() && (s[idx] == '_' || isalpha(s[idx]) || isdigit(s[idx]));++idx);
            return s.substr(tidx,idx - tidx);
        }
        //数字的合法性，在遍历res的时候判定
        string get_int(){
            int tidx = idx;
            for(;idx < s.size() && (isdigit(s[idx]) || isalpha(s[idx]));++idx);
            return s.substr(tidx,idx - tidx);
        }
        string get_symbol(){
            int tidx = idx;
            //注释符号必须被截断
            if(s.substr(idx,2) == "//" || s.substr(idx,2) == "/*"){
                idx += 2;
                return s.substr(tidx,2);
            }
            for(;idx < s.size() && is_symbol(s[idx]);++idx);
            return s.substr(tidx,idx - tidx);
        }
        string get_const_str(){
            int tidx = idx,slash_state = 0;
            for(;idx < s.size();++idx){
                if(s[idx] == '\\')
                    slash_state = !slash_state;
                else if(idx > tidx){
                    if(s[idx] == s[tidx]){//易错点：引号类型应该相同
                        if(!slash_state) break;
                        else slash_state = 0;
                    }
                    else if(slash_state) slash_state = 0;
                }
            }
            ++idx;
            return s.substr(tidx,idx - tidx);
        }
        
        int get_next_token_type(char ch){
            if(ch == '\\') return -1;
            if(ch == '_' || isalpha(ch)) return 1;
            if(isdigit(ch)) return 2;
            if(is_symbol(ch)) return 3;
            if(ch == '\'' || ch == '"') return 4;
            return 0;
        }
        void skip_comment(string symb){
            if(symb == "//"){
                for(;idx < s.size() && s[idx] != '\n';++idx);
                return;
            }
            //注释以"/*"开头的情况
            int comm_state = 0;
            for(;idx < s.size() && s.substr(idx - 2,2) != "*/";++idx);
        }
        bool post_analyze(){
            for(auto rs: res){
                if(rs.code == INTCODE){
                    bool legal = judge_integer(rs.token);
                    if(!legal) return false;
                }
                else if(rs.token[0] == '\''){
                    bool legal = judge_char(rs.token);
                    if(!legal){
                        printf("Multi-character character constant: %s\n",rs.token.c_str());
                        return false;
                    }
                }
            }
            return true;
        }
        bool analyze(){
            idx = 0;
            while(idx < s.size()){
                int typ = get_next_token_type(s[idx]);
                switch(typ){
                    case -1:{
                        puts("Backslash out of constant string.");
                        return false;
                        break;
                    }
                    case 1:{
                        string iden = get_identifier();
                        res.push_back(Node(iden,get_token_id(iden)));
                        break;
                    }
                    case 2:{
                        string d = get_int();
                        res.push_back(Node(d,INTCODE));
                        break;
                    }
                    case 3:{
                        string symb = get_symbol();
                        if(symb == "//" || symb == "/*"){
                            skip_comment(symb);
                        }
                        else{
                            if(symb.size() > 3 || is_symbol_str(symb) == -1){
                                int sl = symbol_dict.get_longest_prefix(symb);
                                idx = idx - symb.size() + sl;//调整idx
                                symb = symb.substr(0,sl);
                                //我们对于未知符号，会自动忽略，比如目前还不支持的#
                            }
                            res.push_back(Node(symb,get_token_id(symb)));
                        }
                        break;
                    }
                    //为了方便，不去掉引号
                    case 4:{
                        string const_str = get_const_str();
                        res.push_back(Node(const_str,get_token_id(const_str)));
                        break;
                    }
                    default:{
                        ++idx;
                        break;
                    }
                }
            }
            return post_analyze();
        }
};

template<typename Type>inline void read(Type &xx){
    Type f = 1;char ch;xx = 0;
    for(ch = getchar();ch < '0' || ch > '9';ch = getchar()) if(ch == '-') f = -1;
    for(;ch >= '0' && ch <= '9';ch = getchar()) xx = xx * 10 + ch - '0';
    xx *= f;
}

const string Lexer::kwd[KWDNUM] = {"int","char","void","if","else","while","for","do","struct","const","return"};
const string Lexer::symbol[SYMNUM] = {"?",":","~","<",">","!=",">=","<=","==","!","&&","||",",",".",";","(",")","[","]","{","}","+","-","*","/","%","&","|","^","<<",">>","++","--","+=","-=","*=","/=","%=","&=","|=","^=","<<=",">>=","="};
const string Lexer::quot[QUONUM] = {"'","\""};
set<char> Lexer::symbol_ch;
Trie Lexer::symbol_dict,Lexer::kwd_dict;

//parser.cpp
//词法分析时已经保证整数合法
int to_int(string d){
    int r = 0;
    if(d[0] == '0'){
        if(d.size() <= 1) return 0;
        else if(d[1] == 'b') re_(i,2,d.size()) r = 2 * r + d[i] - '0';
        else if(d[1] == 'x') re_(i,2,d.size()) r = 16 * r + ('0' <= d[i] && d[i] <= '9' ? d[i] - '0' : (tolower(d[i]) - 'a' + 10));
        else re_(i,1,d.size()) r = 8 * r + d[i] - '0';
    }
    else re_(i,0,d.size()) r = 10 * r + d[i] - '0';
    return r;
}

namespace Inp{
    vector<int> inp;int pos;
    void rd(){
        pos = 0;inp.clear();
        int n;read(n);
        re_(i,0,n){
            int x;read(x);inp.push_back(x);
        }
    }
    int get_inp(){
        if(pos >= inp.size()){
            puts("Warning: input ends, so return to start.");
            pos = 0;
        }
        return inp[pos++];
    }
};

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

//变量自己应知是否是for循环初始化区的变量。
const int FOR_INIT = 1;
struct Array{
    vector<int> vals,sizs;int for_init;
    Array(){}
    Array(vector<int> sizs,int for_init):sizs(sizs),for_init(for_init){
        int totsz = 1;
        for(int siz: sizs) totsz *= siz;
        vals = vector<int>(totsz);
    }
    int get_idx(vector<int> idxs){
        if(idxs.size() != sizs.size()){
            puts("Array dimension number doesn`t match.");
            return -1;
        }
        int idx = 0;
        for(int i = 0;i < sizs.size();++i){
            if(sizs[i] <= idxs[i]){
                printf("Array`s %dth dimension index out of range.\n",i + 1);
                return -1;
            }
            idx = idx * sizs[i] + idxs[i];
        }
        return idx;
    }
    Ret finder(vector<int> idxs){
        int idx = get_idx(idxs);
        if(idx == -1) return Ret(false);
        return Ret(true,vals[idx],0,0);
    }
    Ret setter(vector<int> idxs,int v,string op){
        int idx = get_idx(idxs);
        if(idx == -1) return Ret(false);
        if(op == "=") vals[idx] = v;
        else if(op == "+=") vals[idx] += v;
        else if(op == "-=") vals[idx] -= v;
        else if(op == "*=") vals[idx] *= v;
        else if(op == "/=") vals[idx] /= v;//暂不处理ZeroDivisionError
        else if(op == "%=") vals[idx] %= v;//暂不处理ZeroDivisionError
        else if(op == "<<=") vals[idx] <<= v;
        else if(op == ">>=") vals[idx] >>= v;
        else if(op == "&=") vals[idx] &= v;
        else if(op == "^=") vals[idx] ^= v;
        else if(op == "|=") vals[idx] |= v;
        return Ret(true,vals[idx],0,0);
    }
};

struct Var{
    int v,for_init;
    Var(){}
    Var(int v,int for_init):v(v),for_init(for_init){}
};
//作用域类型(默认建立全局作用域)
const int SCP_GLOBAL = -1,SCP_FUNC = 1,SCP_WHILE = 2,SCP_FOR = 3,SCP_IF = 4,SCP_DOWHILE = 5,SCP_ANONYMOUS = 6;
struct Scope{
    map<string,Var> variables;int typ;
    map<string,Array> arrays;
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
    bool new_array(string name,vector<int> sizs,int for_init = 0){
        if(arrays.count(name)){
            printf("Redeclaration of array: '%s'.\n",name.c_str());
            return false;
        }
        for(int i = 0;i < sizs.size();++i){
            if(sizs[i] <= 1){
                printf("Array '%s'`s %dth size should be at least 2, got %d.\n",name.c_str(),i + 1,sizs[i]);
                return false;
            }
        }
        arrays[name] = Array(sizs,for_init);//新定义数组获得默认值0
        return true;
    }
    //保留在for_init部分声明的变量和数组,其余删除
    void del_vars_forloop(){
        if(typ != SCP_FOR) return;
        for(auto it = variables.begin();it != variables.end();){
            if(it->second.for_init == FOR_INIT) it++;
            else variables.erase(it++);
        }
        for(auto it = arrays.begin();it != arrays.end();){
            if(it->second.for_init == FOR_INIT) it++;
            else arrays.erase(it++);
        }
    }
};
//作用域自动管理器：for循环等不必自行管理作用域
struct ScopeManager{
    vector<Scope> scopes;
    vector<int> idxs_cache;//缓存unit0()所产生的idxs，以备unit10()使用
    void new_scope(int scp_typ){
        scopes.push_back(Scope(scp_typ));
    }
    bool new_var(string name,int val,int for_init = 0){
        return scopes.back().new_var(name,val,for_init);
    }
    bool new_array(string name,vector<int> sizs,int for_init = 0){
        return scopes.back().new_array(name,sizs,for_init);
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
    //重载find_array()，支持“仅判定数组存在性”和“同时要得到值”
    int find_array(string var){
        dwn(i,scopes.size() - 1,1){
            if(scopes[i].arrays.count(var)) return i;
            if(scopes[i].typ == SCP_FUNC) break;
        }
        if(scopes[0].arrays.count(var)) return 0;
        return -1;
    }
    Ret find_array(string var,vector<int> idxs){
        int idx = find_array(var);
        if(idx == -1){
            printf("Array '%s' was not declared in this scope.\n",var.c_str());
            return Ret(false);
        }
        Ret suc = scopes[idx].arrays[var].finder(idxs);
        if(!suc.fl) return suc;
        return Ret(true,suc.val,idx,0);
    }
    Ret mdy_array(string var,vector<int> idxs,int v,string op){//用于赋值语句
        int idx = find_array(var);
        if(idx == -1){
            printf("Array '%s' was not declared in this scope.\n",var.c_str());
            return Ret(false);
        }
        Ret suc = scopes[idx].arrays[var].setter(idxs,v,op);
        if(!suc.fl) return suc;
        return Ret(true,suc.val,idx,0);
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
    //用于parse break;和continue;
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
    //是否“赋值符”
    #define IS_ASSIGN(op) ((op) == "=" || (op) == "+=" || (op) == "-=" || (op) == "*=" || (op) == "/=" || (op) == "%=" || (op) == "<<=" || (op) == ">>=" || (op) == "&=" || (op) == "^=" || (op) == "|=")
    private:
        vector<Node> code;
        FuncManager f_mgr;
        ScopeManager spmgr;
    public:
        Parser(){spmgr.new_scope(SCP_GLOBAL);}
        Parser(vector<Node> lex_unit):code(lex_unit){spmgr.new_scope(SCP_GLOBAL);}
        
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
        //p：1st"["的位置。结束位置是the last "]"的下一个
        Ret parse_square_brackets(int p,vector<int> &sizs){
            if(code[p].token != "["){
                printf("Expected '[' but get token: '%s'\n",code[p].token.c_str());return Ret(false);
            }
            while(true){
                Ret bmatch = match_bracket(p);
                if(!bmatch.fl) return bmatch;
                Ret nx = unit11(p + 1);
                if(!nx.fl) return nx;
                sizs.push_back(nx.val);
                p = bmatch.ed;
                if(code[p].token != "[") break;
            }
            return Ret(true,0,p,0);
        }
        
        //p：分号的位置
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
                    printf("Unsupported param type '%s' of function '%s'\n",code[p].token.c_str(),fun_name.c_str());return false;
                }
                ++p;
                if(code[p].code != 0){
                    printf("Illegal param name '%s' of function '%s'\n",code[p].token.c_str(),fun_name.c_str());return false;
                }
                names.push_back(code[p].token);
                ++p;
                if(code[p].token != "," && code[p].token != ")"){
                    printf("Unexpected token '%s' in definition of function '%s'\n",code[p].token.c_str(),fun_name.c_str());return false;
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
            f_mgr.funs["putchar"] = Func(-1,vector<string>(1,"ch"));
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
        //重名时,优先认为它是函数,随后认为它是数组,最后认为它是变量。
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
                else if(code[p].token == "["){
                    if(spmgr.find_array(var) == -1){
                        printf("Array '%s' was not declared in this scope.\n",var.c_str());return Ret(false);
                    }
                    vector<int> idxs;
                    Ret suc = parse_square_brackets(p,idxs);
                    if(!suc.fl) return suc;
                    Ret val = spmgr.find_array(var,idxs);
                    if(!val.fl) return val;
                    spmgr.idxs_cache = idxs;//缓存
                    return Ret(true,val.val,suc.ed,0);
                }
                //变量
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
        Ret unit10(int p){
            Ret nx = unit9(p);if(!nx.fl) return nx;
            string op = code[nx.ed].token;
            if(!IS_ASSIGN(op)) return nx;
            if(code[p].code != 0){
                printf("lvalue '%s' should be a variable\n",code[p].token.c_str());return Ret(false);
            }
            string var = code[p].token;
            //左侧是数组取下标
            //补丁：不要重复遍历相同区域，直接取缓存idxs_cache即可
            if(code[p + 1].token == "["){
                vector<int> idxs = spmgr.idxs_cache;//保存刚刚得到的缓存内容
                for(++p;code[p].token == "[";){
                    Ret ed_obj = match_bracket(p);
                    if(!ed_obj.fl) return ed_obj;
                    p = ed_obj.ed;
                }
                if(nx.ed == p){
                    Ret nx1 = unit10(nx.ed + 1);
                    if(!nx1.fl) return nx1;
                    Ret mdy = spmgr.mdy_array(var,idxs,nx1.val,op);
                    if(!mdy.fl) return mdy;
                    return Ret(true,mdy.val,nx1.ed,0);
                }
                //parse数组取下标后未到达指定位置
                printf("lvalue required as left operand of assignment: %s\n",op.c_str());return Ret(false);
            }
            //左侧是变量
            if(nx.ed == p + 1){
                Ret nx1 = unit10(nx.ed + 1);
                if(!nx1.fl) return nx1;
                Ret mdy = spmgr.mdy_var(var,nx1.val,op);
                if(!mdy.fl) return mdy;
                return Ret(true,mdy.val,nx1.ed,0);
            }
            //左侧不止单个变量
            printf("lvalue required as left operand of assignment: %s\n",op.c_str());return Ret(false);
        }
        Ret parse_cin(int p){
            if(code[p].token != ">>") return Ret(true,0,p,0);
            string var = code[p + 1].token;
            int val = Inp::get_inp();
            p += 2;
            if(code[p].token == "["){
                vector<int> idxs;
                Ret suc = parse_square_brackets(p,idxs);
                if(!suc.fl) return suc;
                spmgr.mdy_array(var,idxs,val,"=");
                p = suc.ed;
            }
            else if(var == "cin");
            else spmgr.mdy_var(var,val,"=");
            return parse_cin(p);
        }
        Ret parse_cout(int p){
            if(code[p].token != "<<") return Ret(true,0,p,0);
            ++p;
            if(code[p].token == "endl"){
                puts("");
                return parse_cout(p + 1);
            }
            Ret nx = unit10(p);
            if(!nx.fl) return nx;
            printf("%d",nx.val);
            return parse_cout(nx.ed);
        }
        //逗号运算符
        Ret unit11(int p){
            if(code[p].token == "cin") return parse_cin(p + 1);
            if(code[p].token == "cout") return parse_cout(p + 1);
            Ret nx = unit10(p);
            if(!nx.fl) return nx;
            while(code[nx.ed].token == ","){
                nx = unit10(nx.ed + 1);
                if(!nx.fl) return nx;
            }
            return nx;
        }
        //declaration已经支持变量声明并赋值,但数组仅支持声明
        Ret declaration(int p,int for_init = 0){
            string var = code[++p].token;
            /* 暂时留着,因为莫名其妙出现“Expected variable but get token: ”
            cout <<"dec "<<code[p].token<<" "<<code[p].code<<endl;//
            */
            EXPECT_VAR(code[p]);++p;
            if(code[p].token == "["){
                vector<int> sizs;
                Ret suc = parse_square_brackets(p,sizs);
                if(!suc.fl) return suc;
                if(!spmgr.new_array(var,sizs,for_init)) return Ret(false);
                p = suc.ed;
            }
            else{
                int val = 0;
                if(code[p].token == "="){
                    Ret nx = unit10(p + 1);
                    if(!nx.fl) return nx;
                    val = nx.val;
                    p = nx.ed;
                }
                if(!spmgr.new_var(var,val,for_init)) return Ret(false);
            }
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
        Ret while_statement(int p){
            Ret ed_obj = next_statement(p);
            if(!ed_obj.fl) return ed_obj;
            int tp = p + 2;
            for(Ret stmts(true);true;){
                Ret nx(true);
                if(code[tp].token != ")"){//如果不是空判定
                    nx = unit11(tp);if(!nx.fl) return nx;
                    //满足退出条件，指向while结束的地方
                    if(!nx.val) return Ret(true,0,ed_obj.ed,0);
                }
                else nx.ed = tp;
                stmts = statements(nx.ed + 1,ed_obj.ed,SCP_WHILE);//nx.ed + 1是"{"或其他
                if(!stmts.fl) return Ret(false);
                //补丁：退出循环之前不应该重新计算判定条件;运行后当即退出
                if(stmts.cmd == CMD_RETURN || stmts.cmd == CMD_BREAK){
                    if(stmts.cmd == CMD_BREAK) return Ret(true,0,ed_obj.ed,0);
                    return Ret(true,stmts.val,ed_obj.ed,stmts.cmd);
                }
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
                Ret nx(true);
                if(code[tp].token != ";"){//如果不是空判定
                    //tp、nx.ed分别是1st、2nd ";"的下一个位置
                    nx = expres_statement(tp);if(!nx.fl) return nx;
                    //满足退出条件，指向for结束的地方
                    if(!nx.val){
                        spmgr.truly_del_scope();
                        return Ret(true,0,ed_obj.ed,0);
                    }
                }
                else nx.ed = tp + 1;
                stmts = statements(rig_brkt.ed,ed_obj.ed,SCP_FOR,0);//rig_brkt.ed是"{"或其他
                if(!stmts.fl) return Ret(false);
                //补丁：退出循环之前不应该重新计算判定条件;运行3rd之前退出
                if(stmts.cmd == CMD_RETURN || stmts.cmd == CMD_BREAK){
                    spmgr.truly_del_scope();//真实的销毁作用域
                    if(stmts.cmd == CMD_BREAK) return Ret(true,0,ed_obj.ed,0);
                    return Ret(true,stmts.val,ed_obj.ed,stmts.cmd);
                }
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
            else if(token == "cin"){
                Ret ci = parse_cin(p + 1);
                if(!ci.fl) return ci;
                return parse_semicolon(ci.ed);
            }
            else if(token == "cout"){
                Ret co = parse_cout(p + 1);
                if(!co.fl) return co;
                return parse_semicolon(co.ed);
            }
            else if(token == "while") return while_statement(p);
            else if(token == "for") return for_statement(p);
            else if(token == "if") return if_statement(p);
            else if(token == ";") return Ret(true,0,++p,0);//空语句
            else if(token == "{"){//匿名作用域
                Ret bkt = match_bracket(p);
                if(!bkt.fl) return bkt;
                Ret nx = statements(p,bkt.ed,SCP_ANONYMOUS);
                return Ret(nx.fl,nx.val,bkt.ed,nx.cmd);
            }
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
        //lim：结束位置,scp_typ：statements应该知道自己运行的这段语句属于哪个块,op：新建作用域的数量(for循环、run_function使用)
        Ret statements(int p,int lim,int scp_typ,int op = 1){
            if(op) spmgr.new_scope(scp_typ);//新建作用域
            Ret nx(true);
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
        //确保返回类型都是int。不需要提供.ed(位置信息)
        Ret run_function(string name,vector<int> params){
            if(name == "putchar"){//putchar外部已判定
                return Ret(true,putchar(params[0]),0,0);
            }
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
    Inp::rd();
    string s;
    for(char s0[SZ];fgets(s0,SZ,stdin) != NULL;){
        string tmps = s0;
        if(tmps.substr(0,5) == "#incl" || tmps.substr(0,5) == "using") continue;
        s += s0;
    }
    Lexer lxr(s);
    bool legal = lxr.analyze();
    Parser psr(lxr.getRes());
    legal = psr.analyze();
    return 0;
}