/*
    We will be using Chess Game as Example.
*/

#ifndef Chess_Board_Library
#define Chess_Board_Library

#include <iostream>
#include <stack>
#include <mutex>
#include <chrono>
#include <thread>
#include "ChessPiece.h"
#include "Signal.h"
#include "Lock.h"
#include "Database.h"



// BoardSpace Contains Singleton Board Class
// Synchronization Enabled in Board Class
// Listens to Signals : NewBoard, SaveBoard, LoadBoard, DeleteSavedBoard, RunMove
// Emits to Signals : Messages, BoardChanged
// Provides Public Method : RunMove, FastRunMove, UndoMove, (i,j) operator, PrintBoard
// Provides Async Synchronization methods : TakeWriteLock, TakeReadLock, ReleaseWriteLock, ReleaseReadLock
// Allows Taking : ReadLock, WriteLock
namespace BoardSpace
{
    using namespace PieceSpace;
    class Board
    {
        public:

        // board Len, and 2D Array
        static const int SideLen = 8;
        static Piece *board[SideLen][SideLen];
        static int PlayerTurn;



        // to provide read/write synchronization
        static bool BoardConstructed;
        static lockSpace::Lock lock;


        // To support Undo Operation
        static std::stack<Loc> PieceTaken;     // piece lifted of board
        static std::stack<Loc> PiecePlaced;      // piece placed on board
        static int timer;


        public:
        Board(size_t write_id = lock.newID())
        {
            lock.getWriteLock(write_id);

            if(BoardConstructed == false){

                // place Chess Pieces
                board[0][0] = new WhiteRook;
                board[0][1] = new WhiteKnight;
                board[0][2] = new WhiteBishop;
                board[0][3] = new WhiteQueen;
                board[0][4] = new WhiteKing;
                board[0][5] = new WhiteBishop;
                board[0][6] = new WhiteKnight;
                board[0][7] = new WhiteRook;

                board[7][0] = new BlackRook;
                board[7][1] = new BlackKnight;
                board[7][2] = new BlackBishop;
                board[7][3] = new BlackKing;
                board[7][4] = new BlackQueen;
                board[7][5] = new BlackBishop;
                board[7][6] = new BlackKnight;
                board[7][7] = new BlackRook;

                for(int j = 0; j < SideLen; j++){
                    board[1][j] = new WhitePawn;
                    board[6][j] = new BlackPawn;
                    for(int i = 2; i < 6; i++){
                        board[i][j] = new Piece;
                    }
                }

                // Clear All History
                while(PieceTaken.size()){
                  PieceTaken.pop();}
                while(PiecePlaced.size()){
                  PiecePlaced.pop();}


                // Mark Board Constructed
                BoardConstructed = true;

                // Connect Signals
                signalSpace::Save.connect(Save);
                signalSpace::Load.connect(Load);
                signalSpace::DeleteSaved.connect(DeleteSaved);
                signalSpace::New.connect(New);
                signalSpace::RunMove.connect(RunMove);
            }

            lock.releaseWriteLock();
        }

        private:
        static std::string toString()
        {
            std::string boardstr = "";
            for(int i = 0; i < SideLen; i++){
                for(int j = 0; j < SideLen; j++){
                    boardstr += board[i][j]->getName();
                }
            }
            return boardstr;
        }

        static void toBoard(const std::string &boardstr)
        {
            std::string::const_iterator it = boardstr.begin();
            for(int i = 0; i < SideLen; i++){
                for(int j = 0; j < SideLen; j++){
                    std::string t = "";
                    t += *(it++);t += *(it++);

                    *board[i][j] = PieceSpace::PieceFactory::Make(t);
                }
            }
        }

        public:
        static void Save(size_t id)
        {
            lock.getReadLock();
            std::string str = toString();
            std::stack<PieceSpace::Loc> taken = PieceTaken;
            std::stack<PieceSpace::Loc> placed = PiecePlaced;
            lock.releaseReadLock();

            dataSpace::Database data;

            size_t w_id = data.lock.getWriteLock();
            if(data.DataHealthy){

                data.saveBoard(str, id, w_id);
                data.saveStack(taken, placed, id, w_id);
                signalSpace::Messages.emit("saving successful");

            }else{signalSpace::Messages.emit("saving board failed");}

            data.lock.releaseWriteLock();
        }

        static void Load(size_t id)
        {
            std::string str = "";
            std::stack<PieceSpace::Loc> taken;
            std::stack<PieceSpace::Loc> placed;

            dataSpace::Database data;

            data.lock.getReadLock();
            if(data.DataHealthy)
            {
                str = data.readBoard(id);
                taken = data.readStack(id, dataSpace::TAKEN_TY);
                placed = data.readStack(id, dataSpace::PLACED_TY);
            }else{signalSpace::Messages.emit("loading board failed");}

            data.lock.releaseReadLock();

            lock.getWriteLock();
            if(str != ""){
                toBoard(str);
                PieceTaken = taken;
                PiecePlaced = placed;
                if(PiecePlaced.size()){
                        timer = PiecePlaced.top().timer+1;
                        PlayerTurn = -PiecePlaced.top().P.getPlayer();
                }
                signalSpace::Messages.emit("loading successful");
            }
            lock.releaseWriteLock();
        }
        static void DeleteSaved(size_t id)
        {
            dataSpace::Database data;
            size_t w_id = data.lock.getWriteLock();

            if(data.DataHealthy){
                data.saveBoard(PieceSpace::newBoardStr, id, w_id);
                data.saveStack(std::stack<PieceSpace::Loc>(), std::stack<PieceSpace::Loc>(), id, w_id);
            }
            else{signalSpace::Messages.emit("deleting saved board failed");}

            data.lock.releaseWriteLock();
        }
        static void New()
        {
            lock.getWriteLock();
            BoardConstructed = false;
            lock.releaseWriteLock();

            Board();
        }


        private:
        // checks blocking boxes between two pieces
        // returns true, if no piece between (ia,ja) -> (ib,jb)
        static bool checkBetween(int ia, int ja, int ib, int jb)
        {
            int di = ib > ia ? 1 : -1 * (ib < ia);
            int dj = jb > ja ? 1 : -1 * (jb < ja);

            ia += di; ja += dj;
            while(di && (di > 0 ? ia < ib : ia > ib)){
                if(board[ia][ja]->getPlayer()) return false;
                ia += di; ja += dj;
            }

            while(dj && (dj > 0 ? ja < jb : ja > jb)){
                if(board[ia][ja]->getPlayer())  return false;
                ia += di; ja += dj;
            }

            return ia == ib && ja == jb;
        }



        private:
        // checks validity of moves
        // returns true, if Piece at A -> B is valid
        // don't take Player turn in account
        static bool ValidMove(Loc A, Loc B)
        {
            if(A.i < 0 || A.i >= SideLen){return false;}
            if(A.j < 0 || A.j >= SideLen){return false;}
            if(B.i < 0 || B.i >= SideLen){return false;}
            if(B.j < 0 || B.j >= SideLen){return false;}
            if(board[A.i][A.j]->getPlayer() != PlayerTurn){return false;}
            if(A.i == B.i && A.j == B.j){return false;}

            int ans = 0;
            int ia = A.i, ja = A.j;
            int ib = B.i, jb = B.j;
            int pa = board[ia][ja]->getPlayer();
            int pb = board[ib][jb]->getPlayer();
            bool checkBet = checkBetween(ia, ja, ib, jb);


            // Piece Move/Attack Rules
            if(// Knight Move/Attack Rule
               (abs(ia-ib) == 1 && abs(ja-jb) == 2 && pb == 0 && board[ia][ja]->movesKnight()) ||
               (abs(ia-ib) == 1 && abs(ja-jb) == 2 && pb == -pa && board[ia][ja]->attacksKnight()) ||

               // Knight Move/Attack Rule
               (abs(ia-ib) == 2 && abs(ja-jb) == 1 && pb == 0 && board[ia][ja]->movesKnight()) ||
               (abs(ia-ib) == 2 && abs(ja-jb) == 1 && pb == -pa && board[ia][ja]->attacksKnight()) ||

               // Diagonal Move Rule
               (abs(ia-ib)==abs(ja-jb) && pb == 0 && board[ia][ja]->movesAnySpa() && board[ia][ja]->movesDia() && checkBet) ||
               (abs(ib-ia)==abs(ja-jb) && abs(jb-ja)==1 && pb == 0 && board[ia][ja]->movesDia() && checkBet) ||

               // Diagonal Attack Rule
               (abs(ia-ib)==abs(ja-jb) && ib > ia && pb == -pa && board[ia][ja]->attacksAnySpa() && board[ia][ja]->attacksDiaUp() && checkBet) ||
               (abs(ib-ia)==abs(ja-jb) && ib == ia+1 && pb == -pa && board[ia][ja]->attacksDiaUp() && checkBet) ||

               // Diagonal Attack Rule
               (abs(ia-ib)==abs(ja-jb) && ib < ia && pb == -pa && board[ia][ja]->attacksAnySpa() && board[ia][ja]->attacksDiaDown() && checkBet) ||
               (abs(ib-ia)==abs(ja-jb) && ib == ia-1 && pb == -pa && board[ia][ja]->attacksDiaDown() && checkBet) ||

               // Vertical Move Rule
               (ib > ia && jb == ja && pb == 0 && board[ia][ja]->movesAnySpa() && board[ia][ja]->movesVerUp() && checkBet) ||
               (ib == ia+1 && jb == ja && pb == -0 && board[ia][ja]->movesVerUp() && checkBet) ||

               // Vertical Move Rule
               (ib < ia && jb == ja && pb == 0 && board[ia][ja]->movesAnySpa() && board[ia][ja]->movesVerDown() && checkBet) ||
               (ib == ia-1 && jb == ja && pb == 0 && board[ia][ja]->movesVerDown() && checkBet) ||

               // Vertical Attack Rule
               (ib != ia && jb == ja && pb == -pa && board[ia][ja]->attacksAnySpa() && board[ia][ja]->attacksVer() && checkBet) ||
               (abs(ib-ia)==1 && jb == ja && pb == -pa && board[ia][ja]->attacksVer() && checkBet) ||

               // Horizontal Move Rule
               (ib == ia && jb != ja && pb == 0 && board[ia][ja]->movesAnySpa() && board[ia][ja]->movesHor() && checkBet) ||
               (ib == ia && abs(jb-ja)==1 && pb == 0 && board[ia][ja]->movesHor() && checkBet) ||

               // Horizontal Attack Rule
               (ib == ia && jb != ja && pb == -pa && board[ia][ja]->attacksAnySpa() && board[ia][ja]->attacksHor() && checkBet) ||
               (ib == ia && abs(jb-ja)==1 && pb == -pa && board[ia][ja]->attacksHor() && checkBet) ||

               // Fast Start Move Rule
               (ib == ia+2 && jb == ja && pb == 0 && board[ia][ja]->movesVerUp() && !board[ia][ja]->isMoved() && checkBet) ||         // fast start move
               (ib == ia-2 && jb == ja && pb == 0 && board[ia][ja]->movesVerDown() && !board[ia][ja]->isMoved() && checkBet) ||


               // Castling Move Rule
               (ia == ib && abs(ja-jb)==3 && board[ia][ja]->dosCastle() &&
                !board[ia][ja]->isMoved() && !board[ja][jb]->isMoved() && checkBet)                  // castling move

               // Enpassent

               // Promotion
               ){ans = 1;}
            else{
                signalSpace::Messages.emit("Invalid Moved played!!");
            }

            return ans;
        }


        private:
        // Utility Function, to move Pieces around on board
        static void takePiece(Loc A, bool push = true)
        {
            if(push){PieceTaken.push(Loc{A.i, A.j, *board[A.i][A.j], timer});}

            delete board[A.i][A.j];
            board[A.i][A.j] = new Piece;
        }
        static void placePiece(Loc B, Piece P, bool push = true)
        {
            if(push){PiecePlaced.push(Loc{B.i, B.j, P, timer}); }

            if(board[B.i][B.j]->getPlayer()){signalSpace::Debug.emit("Piece move logic error");}

            *board[B.i][B.j] = P;
            if(push){board[B.i][B.j]->Move();}
        }
        static void movePiece(Loc A, Loc B, bool push = true)
        {
            Piece a = *board[A.i][A.j];

            takePiece(A, push);
            takePiece(B, push);
            placePiece(B, a, push);
        }



        public:
        // moves Pieces on Board
        // don't check for validity
        // don't acquire lock
        static void FastRunMove(Loc A, Loc B, size_t id = lock.newID())
        {
            lock.getWriteLock(id);

            // enpassent
            if(B.j-1 >= 0 && board[B.i][B.j-1]->getPlayer() == -board[A.i][A.j]->getPlayer() &&
               PiecePlaced.size() &&
               PiecePlaced.top().i == B.i && PiecePlaced.top().j == B.j &&
               PiecePlaced.top().timer == timer-1){
                   takePiece(Loc{B.i, B.j-1});}

            if(B.j+1 < SideLen && board[B.i][B.j+1]->getPlayer() == -board[A.i][A.j]->getPlayer() &&
               PiecePlaced.size() &&
               PiecePlaced.top().i == B.i && PiecePlaced.top().j == B.j &&
               PiecePlaced.top().timer == timer-1){
                    takePiece(Loc{B.i, B.j+1});}

            // Castling
            if(A.i == B.i && abs(A.j-B.j)==3 && board[A.i][A.j]->dosCastle() &&
                !board[A.i][A.j]->isMoved() && !board[B.i][B.j]->isMoved()){
                        movePiece(B, Loc{A.i, A.j < B.j ? A.j+1 : A.j-1});}

            movePiece(A, B);

            //if(board[B.i][B.j]->getName() == "WP" && B.i == SideLen-1){signal(Promotion, B.i, B.j);}
            //if(board[B.i][B.j]->getName() == "BP" && B.i == 0){signal(Promotion, B.i, B.j);}
            //signal(change);

            timer++;
            PlayerTurn = -PlayerTurn;

            // emit boardchange signal
            signalSpace::BoardChanged.emit();

            lock.releaseWriteLock();
        }

        public:
        // runs piece from A -> B
        // Checks Validity of Move
        // supports Undo
        static void RunMove(Loc A, Loc B, size_t write_id = lock.newID()){

            lock.getWriteLock(write_id);

            if(ValidMove(A, B)){
                FastRunMove(A, B, write_id);
            }

            lock.releaseWriteLock();
        }


        public:
        static void UndoMove(size_t write_id = lock.newID())
        {
            lock.getWriteLock(write_id);

            if(timer > 1)
            {
                timer--;
                while(PiecePlaced.size() && PiecePlaced.top().timer > timer){PiecePlaced.pop();}
                while(PieceTaken.size() && PieceTaken.top().timer > timer){PieceTaken.pop();}


                while(PiecePlaced.size() && PiecePlaced.top().timer == timer){
                    takePiece(PiecePlaced.top(), false);
                    PiecePlaced.pop();
                }
                while(PieceTaken.size() && PieceTaken.top().timer == timer){
                    placePiece(PieceTaken.top(), PieceTaken.top().P, false);
                    PieceTaken.pop();
                }

                PlayerTurn = -PlayerTurn;
            }

            lock.releaseWriteLock();

        }


        public:
        // utility function
        Piece operator()(int i, int j){
            return *board[i][j];
        }

        public:
        // utility func
        static void PrintBoard(){

            lock.getReadLock();

            for(int i = SideLen-1; i >= 0; i--){
                for(int j = 0; j < SideLen; j++){
                    std::cerr << (board[i][j]->getPlayer() ? board[i][j]->getName() : "__") << " ";
                }std::cerr << "\n";
            }

            lock.releaseReadLock();
        }
    };

    Piece* Board::board[Board::SideLen][Board::SideLen];
    bool Board::BoardConstructed;
    int Board::PlayerTurn = 1;

    std::stack<Loc> Board::PieceTaken;
    std::stack<Loc> Board::PiecePlaced;
    int Board::timer = 1;

    lockSpace::Lock Board::lock;
}
#endif
