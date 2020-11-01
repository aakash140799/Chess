
#ifndef gui_Board_library
#define gui_Board_library


#include <iostream>
#include <chrono>
#include <regex>
#include "Board.h"
#include "Signal.h"
#include "signalAction.h"


// guiBoardSpace provides GuiBoard class
// GuiBoard provides console based representation
// of chess board.
// Uses Board class from chess_Board_library
// provides methods : loop
// emits to signals : Messages, RunMove, Destroy, SaveBoard, LoadBoard, NewBoard, DeleteSavedBoard
// !!supposed to be run in new thread
namespace guiBoardSpace
{
    const std::string movStr = "([0-9]+)(,)([0-9]+)(,)([0-9]+)(,)([0-9]+)";
    const std::string quitStr = "Q|q";
    const std::string undoStr = "U|u";
    const std::string saveStr = "(S|s),[0-9]+";
    const std::string loadStr = "(L|l)(,)([0-9]+)";
    const std::string newBoardStr = "N|n";
    const std::string deleteSavedStr = "(D|d)(,)([0-9]+)";
    const std::regex movPat(movStr);
    const std::regex quitPat(quitStr);
    const std::regex undoPat(undoStr);
    const std::regex savePat(saveStr);
    const std::regex loadPat(loadStr);
    const std::regex newBoardPat(newBoardStr);
    const std::regex deleteSavedPat(deleteSavedStr);

    const std::string prompt = "\n1. Enter A move : i1,j1,i2,j2 \n2. Q for quit\n3. U for undo\n4. S,no for saveBoard\n5. L,no for loadboard\n6. N for newboard\n7. D for deleteboard\n";

    class GuiBoard
    {
        BoardSpace::Board board;
        std::string message = "";
        size_t connect_id = 0;

        void clearScreen(){

            system("CLS");
        }

        void printBoard()
        {
            for(int i = board.SideLen-1; i >= 0; i--){
                for(int j = 0; j < board.SideLen; j++){
                    std::cout << (board(i,j).getPlayer() ? board(i,j).getName() : "__");
                    std::cout << " ";
                }
                std::cout << "\n";
            }
        }

        void int_input(std::string::iterator be, std::string::iterator ed, int *inp)
        {
            inp[0] = 0;
            while(be != ed)
            {
                if(*be == ','){
                    inp++;
                    inp[0] = 0;
                }
                else{inp[0] *= 10;
                    inp[0] += (*be) - '0';}
                be++;
            }
        }

        void input(int *inp)
        {
            std::string inps;
            std::cin.ignore(-1);
            std::cin.clear(std::ios::goodbit);
            std::cin >> inps;


            if(std::regex_match(inps, movPat)){inp[0] = 1;int_input(inps.begin(),inps.end(),inp+1);}
            else if(std::regex_match(inps, quitPat)){inp[0] = 2;}
            else if(std::regex_match(inps, undoPat)){inp[0] = 3;}
            else if(std::regex_match(inps, savePat)){inp[0] = 4;int_input(inps.begin()+2, inps.end(),inp+1);}
            else if(std::regex_match(inps, loadPat)){inp[0] = 5;int_input(inps.begin()+2, inps.end(),inp+1);}
            else if(std::regex_match(inps, newBoardPat)){inp[0] = 6;}
            else if(std::regex_match(inps, deleteSavedPat)){inp[0] = 7;int_input(inps.begin()+2, inps.end(),inp+1);}
            else{inp[0] = -1;}
        }

        public:
        void loop()
        {
            while(true)
            {
                if(!board.lock.getAsyncReadLock()){
                    std::cout << ".";
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }else
                {
                    clearScreen();
                    printBoard();

                    board.lock.releaseReadLock();

                    // print messages
                    signalActions::MessageQueue mqueue;
                    std::string str = mqueue.fetch();
                    while(str != ""){
                        std::cout << str << "\n";
                        str = mqueue.fetch();
                    }
                    std::cout << "\n";

                    std::cout << prompt;
                    int inp[6];
                    input(inp);

                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                    if(inp[0] == -1){signalSpace::Messages.emit("Invalid Input");}
                    else if(inp[0] == 1){signalSpace::RunMove.emit(PieceSpace::Loc{inp[1],inp[2]},
                                                       PieceSpace::Loc{inp[3],inp[4]}, board.lock.newID());}
                    else if(inp[0] == 2){signalSpace::Destroy.emit();
                                        break;}
                    else if(inp[0] == 3){board.UndoMove();}
                    else if(inp[0] == 4){signalSpace::Save.emit(inp[1]);}
                    else if(inp[0] == 5){signalSpace::Load.emit(inp[1]);}
                    else if(inp[0] == 6){signalSpace::New.emit();}
                    else if(inp[0] == 7){signalSpace::DeleteSaved.emit(inp[1]);}

                }
            }

        }
    };
}
#endif // gui_Board_library
