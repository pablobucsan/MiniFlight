

#ifndef INSTRUMENTS_H
#define INSTRUMENTS_H

/**
 * DESCRIPTION: High-level abstraction and coordination of instruments
 * 
 * RESPONSIBILITIES: 
 * Translate bus messages into driver-level requests
 * Aggregate data from multiple instruments when needed
 * Publish intrument data and status onto the Software Bus
 */



typedef struct InstrumentsManager{
    int state;
}InstrumentsManager;

void init_instruments_cmpnt();


#endif //INSTRUMENTS_MANAGER_H