#include "computationunit.h"
#include "simulationhandler.h"

ComputationUnit::ComputationUnit(SimulationHandler *h, Emitter *e, Receiver *r, Wall *w) : QObject(h), QRunnable()
{
    m_handler = h;
    m_emitter = e;
    m_receiver = r;
    m_wall = w;
    setAutoDelete(false);
}

void ComputationUnit::run(){
    emit computationStarted();
    m_handler->recursiveReflection(m_emitter, m_receiver, m_wall);
    emit computationFinished();
}
