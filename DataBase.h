

#ifndef Database_Library
#define Database_Library


#include "sqlite3.h"
#include "Signal.h"
#include "Lock.h"
#include <fstream>
#include <cstring>
#include <stack>

// dataSpace namespace contains Database Class
//
// Database Class is a singleton, thread safe class
// provides methods : Destroy, isDataHealthy, readBoard, readStack, saveBoard, saveStack
// emits to signals : Debug,
namespace dataSpace
{
    const std::string chessStore = "CREATE TABLE chessSTORE(id INTEGER PRIMARY KEY, board CHAR(128))";
    const std::string stackStore = "CREATE TABLE stackSTORE(id INTERGER REFERENCES chessSTORE, i INTEGER, j INTEGER, timer INTEGER, type INTEGER, piece CHAR(2), PRIMARY KEY(id, i, j, timer, type))";
    const std::string dbLoc = "Data\\Chess.db";
    const std::string dataLoc = "Data\\";

    const std::string selBoard = "SELECT board from chessSTORE where id == #1;";
    const std::string selStack = "SELECT i, j, timer, piece from stackSTORE where id == #1 AND type == #6 ORDER BY timer ASC;";

    const std::string insBoard = "INSERT INTO chessSTORE(id, board) VALUES(#1, #2);";
    const std::string updBoard = "UPDATE chessSTORE SET board == #2 WHERE id == #1;";
    const std::string delStack = "DELETE FROM stackSTORE where id == #1;";
    const std::string insStack = "INSERT INTO stackSTORE(id, i, j, timer, type, piece) VALUES(#1, #3, #4, #5, #6, #7);";

    const std::string connfail =  "connection to database : failed";
    const std::string filefail =  "Database file connection : failed";
    const std::string validfail = "database validation : failed";
    const std::string newfail =   "Database renew : failed";
    const std::string datfail =   "Database Initialization : failed";
    const std::string tryfix =    "trying database fix";

    const std::string connsucc =  "connection to database : success";
    const std::string filesucc =  "Database file connection : success";
    const std::string validsucc = "database validation : success";
    const std::string newsucc =   "Database renew : success";
    const std::string datsucc =   "Database Initialization : success";

    const std::string tear = "tearing connection";
    const std::string mxid = "board id too big";

    const size_t maxBoardCnt = 25;
    const size_t TAKEN_TY = 0;
    const size_t PLACED_TY = 1;
    #define fname(str) str+" : "+__FUNCTION__

    using signalSpace::Debug;
    using PieceSpace::Loc;
    using PieceSpace::PieceFactory;

    class Database
    {
        public:
        static sqlite3* DB;
        static bool DataHealthy;
        static lockSpace::Lock lock;                   // database lock


        public:
        Database()
        {
            size_t w_id = lock.getWriteLock();

            if(connect() && ValidDataFile()){
                Debug.emit(fname(datsucc));
                DataHealthy = true;

            }else if(!ValidDataFile()){

                Debug.emit(fname(tryfix));

                reNewDataBase(w_id);
                if(connect() && ValidDataFile()){
                    Debug.emit(fname(datsucc));
                    DataHealthy = true;
                }
                else{
                    Debug.emit(fname(datfail));
                }

            }

            lock.releaseWriteLock();
        }

        private:
        static bool connect()
        {
            if(DB){tearConn();}

            int rc = sqlite3_open(dbLoc.c_str(), &DB);
            if(rc != SQLITE_OK){
                Debug.emit(fname(connfail));

                tearConn();
                return false;
            }

            Debug.emit(fname(connsucc));
            return true;
        }
        static void tearConn()
        {
            if(DB){
                Debug.emit(fname(tear));

                sqlite3_close(DB);
                DB = 0;
                DataHealthy = false;
            }
        }

        public:
        static void Destroy(int write_lock_id = lock.newID()){

            lock.getWriteLock(write_lock_id);
            tearConn();
            lock.releaseWriteLock();
        }

        public:
        static bool isDataHealthy(){

            lock.getReadLock();
            bool state = DataHealthy;
            lock.releaseReadLock();

            return state;
        }

        private:
        static bool ValidDataFile()
        {
            if(DB == 0){return false;}

            int cnt = 0;
            char *errmsg = 0;
            sqlite3_exec(DB, "SELECT sql from sqlite_schema", [](void *dat, int argc, char **argv, char **column) -> int{
                            int *cnt = (int *)dat;
                            for(int i = 0; i < argc; i++){
                                if(argv[i]){
                                std::string t = argv[i];
                                if(t == chessStore || t == stackStore){(*cnt)++;}}
                            }
                            return 0;
                         }, &cnt, &errmsg);

            bool valid = cnt == 2;
            if(errmsg){
                Debug.emit(fname(std::string(errmsg)));
                Debug.emit(fname(validfail));
            }
            else{
                Debug.emit(fname(validsucc));
                Debug.emit(std::string("valid:")+(valid?"1":"0"));
            }

            return valid;
        }

        private:
        static void reNewDataBase(size_t w_id = lock.newID())
        {
            std::fstream file(dbLoc, std::ios::out);
            if(file){
                Debug.emit(fname(filesucc));
                file.close();
            }else{
                Debug.emit(fname(filefail));
            }


            if(connect()){
                char *errmsg = 0;
                bool err = false;

                sqlite3_exec(DB, chessStore.c_str(), 0, 0, &errmsg);
                if(errmsg){
                    Debug.emit(fname(std::string(errmsg)));
                    err = true;
                }

                sqlite3_exec(DB, stackStore.c_str(), 0, 0, &errmsg);
                if(errmsg){
                    Debug.emit(fname(std::string(errmsg)));
                    err = true;
                }

                if(err){
                    Debug.emit(fname(newfail));
                }
                else{
                    Debug.emit(fname(newsucc));
                    for(int i = 0; i <= maxBoardCnt; i++){
                        sqlite3_exec(DB, buildSql(insBoard, i, PieceSpace::newBoardStr).c_str(), 0, 0, &errmsg);
                        if(errmsg){
                            Debug.emit(std::string(errmsg)+__FUNCTION__);
                            err = true;
                        }
                    }
                }
            }

            tearConn();
        }

        private:
        static std::string buildSql(std::string sql, size_t id = 0, std::string board = "", Loc loc = Loc{-1,-1}, size_t type = 0){
            std::string tsql = "";

            std::string ids = std::to_string(id);
            std::string tys = std::to_string(type);
            std::string li = std::to_string(loc.i);
            std::string lj = std::to_string(loc.j);
            std::string tm = std::to_string(loc.timer);

            for(size_t i = 0, n = sql.size(); i < n; i++){
                if(sql[i] != '#'){tsql.push_back(sql[i]);}
                else{

                    int c = sql[i+1]-'0';
                    switch(c){
                        case 1: tsql += ids; break;
                        case 2: tsql += "'"+board+"'"; break;
                        case 3: tsql += li; break;
                        case 4: tsql += lj; break;
                        case 5: tsql += tm; break;
                        case 6: tsql += tys; break;
                        case 7: tsql += "'"+loc.P.getName()+"'";
                    }
                    i++;
                }
            }

            return tsql;
        }

        public:
        static std::string readBoard(size_t id)
        {
            if(id > maxBoardCnt){return "";}

            std::string board = "";
            lock.getReadLock();
            if(isDataHealthy())
            {
                char *errmsg = 0;
                sqlite3_exec(DB, buildSql(selBoard, id).c_str(),
                             [](void *dat, int argc, char** argv, char **column) -> int{
                                std::string *board = (std::string *)dat;
                                if(argc && argv[0]){*board = argv[0];}

                                return 0;

                             }, &board, &errmsg);
                if(errmsg){
                    Debug.emit(fname(std::string(errmsg)));
                }
            }
            lock.releaseReadLock();

            return board;
        }

        public:
        static std::stack<Loc> readStack(size_t id, size_t ty)
        {
            if(id > maxBoardCnt){return std::stack<Loc>();}

            lock.getReadLock();
            std::stack<Loc> stk;
            if(isDataHealthy()){
                char *errmsg = 0;
                sqlite3_exec(DB, buildSql(selStack, id, "", Loc{}, ty).c_str(),
                        [](void *dat, int argc, char **argv, char **column) -> int{
                            std::stack<Loc> *stk = (std::stack<Loc> *)dat;

                            Loc p;
                            p.i = atoi(argv[0]);
                            p.j = atoi(argv[1]);
                            p.timer = atoi(argv[2]);
                            p.P = PieceFactory::Make(std::string(argv[3]));

                            stk->push(p);
                            return 0;

                        }, &stk, &errmsg);

                if(errmsg){
                    Debug.emit(std::string(errmsg));
                }
            }
            lock.releaseReadLock();

            return stk;
        }

        public:
        static void saveBoard(std::string board, const size_t id, const size_t w_id = lock.newID())
        {
            if(id > maxBoardCnt){
                    Debug.emit(fname(mxid));
                    return;
            }

            lock.getWriteLock(w_id);
            if(DataHealthy)
            {
                char *errmsg = 0;

                sqlite3_exec(DB, buildSql(updBoard, id, board).c_str(), 0, 0, &errmsg);
                if(errmsg){
                    Debug.emit(std::string(errmsg));
                }else{Debug.emit(std::string("Board Saved!!"));}
            }

            lock.releaseWriteLock();
        }

        // taken, placed must be saved in single function call
        // because func erases previous stacks to make sure
        // duplicates don't exist
        static void saveStack(std::stack<Loc> taken, std::stack<Loc> placed, const size_t id, size_t w_id = lock.newID())
        {
            if(id > maxBoardCnt){
                    Debug.emit(fname(mxid));
                    return;
            }

            lock.getWriteLock(w_id);

            char *errmsg = 0;
            bool err = false;
            if(DataHealthy)
            {
                if(!err){
                    sqlite3_exec(DB, buildSql(delStack, id).c_str(), 0, 0, &errmsg);

                    if(errmsg){
                        Debug.emit(fname(std::string(errmsg)));
                        err = true;
                    }
                }

                if(!err){
                    while(taken.size()){
                        sqlite3_exec(DB, buildSql(insStack, id, "", taken.top(), TAKEN_TY).c_str(), 0, 0, &errmsg);
                        if(errmsg){
                            Debug.emit(fname(std::string(errmsg)));
                            err = true;
                            break;
                        }
                        taken.pop();
                    }
                }

                if(!err){
                    while(placed.size()){
                        sqlite3_exec(DB, buildSql(insStack, id, "", placed.top(), PLACED_TY).c_str(), 0, 0, &errmsg);
                        if(errmsg){
                            Debug.emit(fname(std::string(errmsg)));
                            err = true;
                            break;
                        }
                        placed.pop();
                    }
                }

                if(!err){
                    Debug.emit("Stack Saved!!");
                }
            }

            lock.releaseWriteLock();
        }
    };

    sqlite3* Database::DB = 0;
    bool Database::DataHealthy = false;
    lockSpace::Lock Database::lock;

}
#endif // database_library

