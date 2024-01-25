#include"tcp_server.h"
#include"rocket/common/log.h"

namespace rocket{

TcpServer::TcpServer(NetAddr::s_ptr local_ptr): m_local_ptr(local_ptr){
    init();
    INFOLOG("rocket TcpServer listen success on [%s]", m_local_ptr->toString().c_str());
}

TcpServer::~TcpServer(){
    if(m_main_event_loop){
        delete m_main_event_loop;
        m_main_event_loop = NULL;
    }
    if (m_io_thread_group) {
    delete m_io_thread_group;
    m_io_thread_group = NULL; 
  }
  if (m_listen_fd_event) {
    delete m_listen_fd_event;
    m_listen_fd_event = NULL;
  }
}

void TcpServer::onAccept(){
    int client_fd = m_acceptor->accept();
    m_client_counts++;
    //吧clientfd添加到任意一个IO线程里面

    INFOLOG("TcpServer succ get client, fd=%d",client_fd);
}

void TcpServer::init(){
    m_acceptor = std::make_shared<TcpAcceptor>(m_local_ptr);
    m_main_event_loop = EventLoop::GetCurrentEventLoop();
    m_io_thread_group = new IOThreadGroup(2);
    m_listen_fd_event = new FdEvent(m_acceptor->getListenFd());
    m_listen_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpServer::onAccept, this));
    m_main_event_loop->addEpollEvent(m_listen_fd_event);
}

void TcpServer::start(){
    m_io_thread_group->start();
    m_main_event_loop->loop();
} 
}