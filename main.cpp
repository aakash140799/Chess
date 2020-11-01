
#include "C:\\users\\root\\Desktop\\ChessPiece.h"
#include "C:\\users\\root\\Desktop\\Signal.h"
#include "C:\\users\\root\\Desktop\\Lock.h"
#include "C:\\users\\root\\Desktop\\DataBase.h"
#include "C:\\users\\root\\Desktop\\Board.h"
#include "C:\\users\\root\\Desktop\\GuiBoard.h"
#include "C:\\users\\root\\Desktop\\signalAction.h"

int main()
{
    // initiate
    signalActions::LogAction logs;
    logs.log("--------------------------------------------------------------");
    logs.log("STARTUP\n");

    signalActions::MessageQueue mqueue;
    logs.log("MESSAGE QUEUE STARTUP\n");

    signalSpace::Debug.emit("BOARD STARTUP\n");
    BoardSpace::Board board;

    signalSpace::Debug.emit("DATABASE STARTUP\n");
    dataSpace::Database data;

    signalSpace::Debug.emit("GUIBOARD STARTUP\n");
    guiBoardSpace::GuiBoard gui;

    gui.loop();
    logs.log("ENDUP\n");
}
