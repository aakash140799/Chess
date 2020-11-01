/*
    We will be using Chess Game as Example.
*/

#ifndef Chess_Piece_Library
#define Chess_Piece_Library

#include <string>
// Chess Piece namespace
// contains Piece generic class
// All other Piece class inherit from Piece class
namespace PieceSpace
{
    // generic piece class
    class Piece
    {
        protected:
        bool attackVer = false;
        bool attackHor = false;
        bool attackDiaUp = false;
        bool attackDiaDown = false;
        bool attackAnySpa = false;
        bool attackKnight = false;

        bool moveVerUp = false;
        bool moveVerDown = false;
        bool moveDia = false;
        bool moveHor = false;
        bool moveAnySpa = false;
        bool moveKnight = false;

        bool doCastle = false;
        bool doEnpass = false;
        bool doPromote = false;
        float utility = 0;
        bool moved = false;

        int player = 0;
        std::string name = "NA";

        public:
        bool attacksVer(){return attackVer;}
        bool attacksHor(){return attackHor;}
        bool attacksDiaUp(){return attackDiaUp;}
        bool attacksDiaDown(){return attackDiaDown;}
        bool attacksAnySpa(){return attackAnySpa;}
        bool attacksKnight(){return attackKnight;}
        bool movesVerUp(){return moveVerUp;}
        bool movesVerDown(){return moveVerDown;}
        bool movesDia(){return moveDia;}
        bool movesHor(){return moveHor;}
        bool movesAnySpa(){return moveAnySpa;}
        bool movesKnight(){return moveKnight;}
        bool dosCastle(){return doCastle;}
        bool dosEnpass(){return doEnpass;}
        bool dosPromote(){return doPromote;}
        float getUtility(){return utility;}
        int isMoved(){return moved;}
        void Move(){moved=true;}
        int getPlayer(){return player;}
        std::string getName(){return name;}
    };

    class WhitePawn : public Piece
    {
        public:
        WhitePawn(){
            Piece();
            moveVerUp = true;
            attackDiaUp = true;
            doEnpass = true;
            doPromote = true;
            utility = 4.0/15;

            player = 1;
            name = "WP";
        }
    };

    class BlackPawn : public Piece
    {
        public:
        BlackPawn(){
            moveVerDown = true;
            attackDiaDown = true;
            doEnpass = true;
            doPromote = true;
            utility = 4.0/15;

            player = -1;
            name = "BP";
        }
    };

    class WhiteRook : public Piece
    {
        public:
        WhiteRook(){
            moveVerUp = true;
            moveVerDown = true;
            moveHor = true;
            attackVer = true;
            attackHor = true;
            moveAnySpa = true;
            attackAnySpa = true;
            utility = 7.0/15;

            player = 1;
            name = "WR";
        }
    };

    class BlackRook : public Piece
    {
        public:
        BlackRook(){
            moveVerUp = true;
            moveVerDown = true;
            moveHor = true;
            attackVer = true;
            attackHor = true;
            moveAnySpa = true;
            attackAnySpa = true;
            utility = 7.0/15;

            player = -1;
            name = "BR";
        }
    };

    class WhiteKnight : public Piece
    {
        public:
        WhiteKnight(){
            moveKnight = true;
            attackKnight = true;
            utility = 10.0/15;

            player = 1;
            name = "WH";
        }
    };

    class BlackKnight : public Piece
    {
        public:
        BlackKnight(){
            moveKnight = true;
            attackKnight = true;
            utility = 10.0/15;

            player = -1;
            name = "BH";
        }
    };

    class WhiteBishop : public Piece
    {
        public:
        WhiteBishop(){
            moveDia = true;
            attackDiaUp = true;
            attackDiaDown = true;
            moveAnySpa = true;
            attackAnySpa = true;
            utility = 6.0/15;

            player = 1;
            name = "WB";
        }
    };

    class BlackBishop : public Piece
    {
        public:
        BlackBishop(){
            moveDia = true;
            attackDiaUp = true;
            attackDiaDown = true;
            moveAnySpa = true;
            attackAnySpa = true;
            utility = 6.0/15;

            player = -1;
            name = "BB";
        }
    };

    class WhiteQueen : public Piece
    {
        public:
        WhiteQueen(){
            moveHor = true;
            moveVerUp = true;
            moveVerDown = true;
            moveDia = true;
            moveAnySpa = true;

            attackHor = true;
            attackVer = true;
            attackDiaUp = true;
            attackDiaDown = true;
            attackAnySpa = true;
            utility = 10.0/15;

            player = 1;
            name = "WQ";
        }
    };

    class BlackQueen : public Piece
    {
        public:
        BlackQueen(){
            moveHor = true;
            moveVerUp = true;
            moveVerDown = true;
            moveDia = true;
            moveAnySpa = true;

            attackHor = true;
            attackVer = true;
            attackDiaUp = true;
            attackDiaDown = true;
            attackAnySpa = true;
            utility = 10.0/15;

            player = -1;
            name = "BQ";
        }
    };

    class WhiteKing : public Piece
    {
        public:
        WhiteKing(){
            moveVerUp = true;
            moveVerDown = true;
            moveHor = true;
            moveDia = true;

            attackVer = true;
            attackHor = true;
            attackDiaUp = true;
            attackDiaDown = true;
            doCastle = true;
            utility = 1;

            player = 1;
            name = "WK";
        }
    };

    class BlackKing : public Piece
    {
        public:
        BlackKing(){
            moveVerUp = true;
            moveVerDown = true;
            moveHor = true;
            moveDia = true;

            attackVer = true;
            attackHor = true;
            attackDiaUp = true;
            attackDiaDown = true;
            doCastle = true;
            utility = 1;

            player = -1;
            name = "BK";
        }
    };

    class PieceFactory
    {
        public:
        static Piece Make(std::string name)
        {
            Piece *p;
            if(name == "WP"){p = new WhitePawn;}
            else if(name == "BP"){p = new BlackPawn;}
            else if(name == "WR"){p = new WhiteRook;}
            else if(name == "BR"){p = new BlackRook;}
            else if(name == "WH"){p = new WhiteKnight;}
            else if(name == "BH"){p = new BlackKnight;}
            else if(name == "WB"){p = new WhiteBishop;}
            else if(name == "BB"){p = new BlackBishop;}
            else if(name == "WQ"){p = new WhiteQueen;}
            else if(name == "BQ"){p = new BlackQueen;}
            else if(name == "WK"){p = new WhiteKing;}
            else if(name == "BK"){p = new BlackKing;}
            else{p = new Piece;}

            Piece P = *p;
            delete p;

            return P;
        }
        static bool isPiece(std::string name)
        {
            return   (name == "WP" ||
                      name == "BP" ||
                      name == "WR" ||
                      name == "BR" ||
                      name == "BH" ||
                      name == "WB" ||
                      name == "BB" ||
                      name == "WQ" ||
                      name == "BQ" ||
                      name == "WK" ||
                      name == "BK");
        }
    };

    const std::string newBoardStr = "WRWHWBWQWKWBWHWRWPWPWPWPWPWPWPWPNANANANANANANANANANANANANANANANANANANANANANANANANANANANANANANANABPBPBPBPBPBPBPBPBRBHBBBKBQBBBHBR";


    struct Loc
    {
        int i, j;
        Piece P;
        int timer;
    };
}

#endif // Chess_Piece_Library
