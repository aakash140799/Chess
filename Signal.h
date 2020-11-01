
//Original Version Written by @codebrainz
//https://gist.github.com/codebrainz/e793c9c716a0d1b1e06b


#ifndef Signal_Library
#define Signal_Library

#include <iostream>
#include <atomic>
#include <cstdint>
#include <functional>
#include <vector>
#include <thread>
#include "ChessPiece.h"


// Signal Class for Asynchronous behavior
// Defines Signal Objects: Messages, RunMove, BoardChanged, SaveBoard, LoadBoard,
//                          DeleteSavedBoard, NewBoard
namespace signalSpace
{
    template<class... Args>
    class Signal
    {
        public:
        typedef std::function<void(Args...)> handler_type;
        typedef std::uint64_t id_type;


        id_type connect(handler_type hnd)
        {
            id_type id(next_id());
            handlers.push_back(Handler{id, hnd});
            return id;
        }

        bool disconnect(id_type id)
        {
            for(size_t i = 0, n = handlers.size(); i < n; i++){
                if(handlers[i].id == id){

                    handlers.erase(handlers.begin() + i);
                    return true;
                }
            }

            return false;
        }

        void emit(Args... args)
        {
            for(Handler &h : handlers){
                h.func(args...);
            }
            /*
            std::vector<std::thread> threadPool(handlers.size());
            for(size_t i = 0, n = handlers.size(); i < n; i++){
                threadPool[i] = std::thread{handlers[i].func, args...};
            }

            for(auto &t : threadPool){
                t.join();
            }*/
        }


        public:
        struct Handler
        {
            id_type id;
            handler_type func;
        };

        std::vector<Handler> handlers;

        static next_id(){

            static std::atomic<id_type> id(0);
            return ++id;
        }
    };

    Signal<std::string> Messages;
    Signal<std::string> Debug;

    Signal<PieceSpace::Loc, PieceSpace::Loc, size_t> RunMove;
    Signal<> BoardChanged;
    Signal<size_t> Save;
    Signal<size_t> Load;
    Signal<size_t> DeleteSaved;
    Signal<> New;
    Signal<> Destroy;
}
#endif // Signal_Library

