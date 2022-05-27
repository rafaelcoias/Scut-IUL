/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 2 de Sistemas Operativos
 **
 ** Aluno: Nº: 105361      Nome: Rafael Cóias
 ** Nome do Módulo: servidor.c versão 3
 ** Descrição/Explicação do Módulo: 
 **
 **
 ******************************************************************************/
#include "common.h"
#include "utils.h"
// #define DEBUG_MODE FALSE                             // To disable debug messages, uncomment this line

/* Variáveis globais */
int shmId;                                              // Variável que tem o ID da Shared Memory
int msgId;                                              // Variável que tem o ID da Message Queue
int semId;                                              // Variável que tem o ID do Array de Semáforos
Mensagem mensagem;                                      // Variável que tem o pedido enviado do Cliente para o Servidor
DadosServidor *dadosServidor;                           // Variável que vai ficar com a memória partilhada
int indice_lista;                                       // Índice corrente da Lista, que foi reservado pela função reservaEntradaBD()
struct sembuf aux;

/* Protótipos de funções */
int shmGet();                                           // S1:   Função a ser implementada pelos alunos
int shmCreateAndInit();                                 // S2:   Função a ser implementada pelos alunos
int loadStats( Contadores* );                           // S2.3: Função a ser implementada pelos alunos
int createIPC();                                        // S3:   Função a ser implementada pelos alunos
Mensagem recebePedido();                                // S4:   Função a ser implementada pelos alunos
int criaServidorDedicado();                             // S5:   Função a ser implementada pelos alunos
void trataSinalSIGINT( int );                           // S6:   Função a ser implementada pelos alunos
int sd_armaSinais();                                    // SD7:  Função a ser implementada pelos alunos
int sd_validaPedido( Mensagem );                        // SD8:  Função a ser implementada pelos alunos
int sd_reservaEntradaBD( DadosServidor*, Mensagem );    // SD9:  Função a ser implementada pelos alunos
int sd_apagaEntradaBD( DadosServidor*, int );           //       Função a ser implementada pelos alunos
int sd_iniciaProcessamento( Mensagem );                 // SD10: Função a ser implementada pelos alunos
int sd_sleepRandomTime();                               // SD11: Função a ser implementada pelos alunos
int sd_terminaProcessamento( Mensagem );                // SD12: Função a ser implementada pelos alunos
void sd_trataSinalSIGHUP( int );                        // SD13: Função a ser implementada pelos alunos
void sd14(int sem_num, int sem_op);                     // SD14: Função a ser implementada pelos alunos

int main() {    // Não é suposto que os alunos alterem nada na função main()
    // S1
    if ( !shmGet() ) {
        // S2
        shmCreateAndInit();
    }
    // S3
    createIPC();

    while ( TRUE ) {  // O processamento do Servidor é cíclico e iterativo
        // S4
        mensagem = recebePedido();
        // S5
        int pidFilho = criaServidorDedicado();
        if ( !pidFilho ) {  // Processo Servidor Dedicado - Filho
            // SD7
            sd_armaSinais();
            // SD8
            sd_validaPedido( mensagem );
            // SD9
            indice_lista = sd_reservaEntradaBD( dadosServidor, mensagem );
            // SD10
            sd_iniciaProcessamento( mensagem );
            // SD11
            sd_sleepRandomTime();
            // SD12
            sd_terminaProcessamento( mensagem );
        }
    }
}

/**
 * @brief Utility to Display the values of the shared memory
 * 
 * @param shm Shared Memory
 * @param ignoreInvalid Do not display the elements that have the default value
 */
void shmView( DadosServidor* shm, int ignoreInvalid ) {
    printf( "Conteúdo da SHM Contadores: Normal: %d | Via Verde: %d | Anomalias: %d\n", shm->contadores.contadorNormal, shm->contadores.contadorViaVerde, shm->contadores.contadorAnomalias );
    printf( "Conteúdo da SHM Passagens:\n" );
    for ( int i = 0; i < NUM_PASSAGENS; ++i ) {
        if ( !ignoreInvalid || -1 != shm->lista_passagens[i].tipo_passagem ) {
            printf( "Posição %2d: %6d | %-9s | %-20s | %d\n", i, shm->lista_passagens[i].tipo_passagem, shm->lista_passagens[i].matricula, shm->lista_passagens[i].lanco, shm->lista_passagens[i].pid_cliente );
        }
    }
}

/**
 *  O módulo Servidor de Passagens é responsável pelo processamento de pedidos de passagem que chegam ao sistema Scut-IUL.
 *  Este módulo é, normalmente, o primeiro dos dois (Cliente e Servidor) a ser executado, e deverá estar sempre ativo,
 *  à espera de pedidos de passagem. O tempo de processamento destes pedidos varia entre os MIN_PROCESSAMENTO segundos
 *  e os MAX_PROCESSAMENTO segundos. Findo esse tempo, este módulo sinaliza ao condutor de que a sua passagem foi processada.
 *  Este módulo deverá possuir contadores de passagens por tipo, um contador de anomalias e uma lista com capacidade para
 *  processar NUM_PASSAGENS passagens. O módulo Servidor de Passagens é responsável por realizar as seguintes tarefas:
 */

/**
 * S1   Tenta abrir uma memória partilhada (shared memory) IPC que tem a KEY IPC_KEY definida em common.h
 *      (alterar esta KEY para ter o valor do nº do aluno, como indicado nas aulas).
 *      Se essa memória partilhada ainda não existir passa para o passo S2 sem erro. Caso contrário, liga-se a ela.
 *      Em caso de erro, dá error S1 "<Problema>", e termina o processo Servidor com exit code -1.
 *      Senão, dá success S1 "Abri Shared Memory já existente com ID <shmId>"
 *      --> Preenche as variáveis globais shmId e dadosServidor.
 *
 * @return int 0 se a memória partilhada ainda não existe no sistema ou 1 se a memória partilhada já existe no sistema
 */
int shmGet() {
    debug("S1 <");

    int shm;
    
    shm = shmget(IPC_KEY, sizeof(DadosServidor), 0);
    if (shm != -1)
    {
        shmId = shm;
        dadosServidor = shmat(shmId, 0, 0);
        if (!dadosServidor)
        {
            error("S1", "Erro ao abrir a Shared Memory.");
            exit(-1);
        }
        success("S1", "Abri Shared Memory já existente com ID %i", shmId);
    }
    else if (ENOENT != errno)
    {
        error("S1", "Shared Memory não existente.");
        exit(-1);
    }

    debug("S1 >");
    return ( shmId > 0 );
}

/**
 * S2   Se no ponto S1 a memória partilhada ainda não existia, então realiza as seguintes operações:
 *      S2.1    Cria uma memória partilhada com a KEY IPC_KEY definida em common.h e com o tamanho para conter os Dados do Servidor,
 *              e liga-se a ela. Em caso de erro, dá error S2.1 "<Problema>", e termina o processo Servidor com exit code -1.
 *              Caso contrário, dá success S2.1 "Criei Shared Memory com ID <shmId>";
 *              --> Preenche as variáveis globais shmId e dadosServidor;
 *
 *      S2.2    Inicia a lista de passagens, preenchendo em todos os elementos o campo tipo_passagem=-1 (“Limpa” a lista de passagens).
 *              Em caso de qualquer erro, dá error S2.2 "<Problema>", e termina o processo Servidor com exit code -1.
 *              Caso contrário, dá success S2.2 "Iniciei Shared Memory Passagens";
 *
 *      S2.3    (realizado na função loadStats()): Tem 3 contadores, para as passagens Normal, Via Verde e passagens com anomalia.
 *              Se o ficheiro FILE_STATS existir na diretoria local, abre-o e lê os seus dados (em formato binário, ver formato em S6.2)
 *              e carrega o valor nos contadores. Se houver erro na leitura do ficheiro, dá error S2.3 "<Problema>", 
 *              e termina o Servidor com exit code -1. Caso contrário, dá success S2.3 "Estatísticas Carregadas".
 *              Se o ficheiro não existir, inicia os três contadores com o valor 0 e dá success S2.3 "Estatísticas Iniciadas";
 *
 * @return int shmId
 */
int shmCreateAndInit() {
    debug("S2 <");

    //S2.1

    shmId = shmget(IPC_KEY, sizeof(DadosServidor), IPC_CREAT | 0600);
    dadosServidor = shmat(shmId, 0, 0);
    if (shmId == -1)
    {
        error("S2.1", "Erro ao criar memória partilhada");
        exit(-1);
    }
    success("S2.1", "Criei Shared Memory com ID %i", shmId);

    //S2.2

    for (int i = 0; i != NUM_PASSAGENS; i++)
        dadosServidor->lista_passagens[i].tipo_passagem = -1;
    success("S2.2", "Iniciei Shared Memory Passagens");

    //S2.3

    loadStats(&dadosServidor->contadores);

    debug("S2 >");
    return shmId;
}

/**
 *      S2.3    Tem 3 contadores, para as passagens Normal, Via Verde e passagens com anomalia.
 *              Se o ficheiro FILE_STATS existir na diretoria local, abre-o e lê os seus dados (em formato binário, ver formato em S6.2)
 *              e carrega o valor nos contadores. Se houver erro na leitura do ficheiro, dá error S2.3 "<Problema>", 
 *              e termina o Servidor com exit code -1. Caso contrário, dá success S2.3 "Estatísticas Carregadas".
 *              Se o ficheiro não existir, inicia os três contadores com o valor 0 e dá success S2.3 "Estatísticas Iniciadas";
 *
 * @return int Sucesso
 */
int loadStats( Contadores* pStats ) {
    debug("S2.3 <");

    if (!access(FILE_STATS, F_OK))
    {
        FILE *fd = fopen(FILE_STATS, "r");
        if (fread(&pStats->contadorNormal, sizeof(int), 1, fd) < 1 || fread(&pStats->contadorViaVerde, sizeof(int), 1, fd) < 1 || fread(&pStats->contadorAnomalias, sizeof(int), 1, fd) < 1)
        {
            error("S2.3", "Erro ao ler o ficheiro %s", FILE_STATS);
            exit(-1);
        }
        success("S2.3", "Estatísticas Carregadas");
        fclose(fd);
    }
    else
    {
        pStats->contadorNormal = 0;
        pStats->contadorViaVerde = 0;
        pStats->contadorAnomalias = 0;
        success("S2.3", "Estatísticas Iniciadas");
    }

    debug("S2.3 >");
    return 0;
}

/**
 * S3   Cria uma message queue com a KEY IPC_KEY definida em common.h,
 *      Se a message queue já existir, apaga-a e cria de novo, 
 *      --> Preenche a variável global msgId.
 *
 *      Arma os sinais SIGINT (ver S6) e SIGCHLD (programa para ignorar este sinal).
 *      Se houver erros, dá error S3 "<Problema>" e termina o Servidor com exit code -1.
 *      Caso contrário, dá success S3 "Criei mecanismos IPC";
 *
 * @return int msgId
 */
int createIPC() {
    debug("S3 <");

    ushort init[] = {1, 1};

    //SEMAPHORES
    semId = semget(IPC_KEY, 2, IPC_CREAT | 0666);
    if(semId == -1 || semctl(semId, 0, SETALL, init) == -1)
        error("S3", "Erro ao criar/iniciar os Semáforos");

    //MESSAGES
    msgId = msgget(IPC_KEY, IPC_CREAT | IPC_EXCL | 0666);
    if(msgId != -1)
        msgctl(msgId, IPC_RMID, NULL);
    else
        error("S3", "Não foi possível criar a Message Queue");
    
    //INIT
    signal(SIGINT, trataSinalSIGINT);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    success("S3", "Criei mecanismos IPC");

    debug("S3 >");
    return 0;
}

/**
 * S4   Lê a informação da message queue numa mensagem com o tipo de mensagem 1.
 *      Essa mensagem deverá ter a action 1 – Pedido e deverá conter um elemento do tipo Passagem.
 *      Se houver erro na operação, dá error S4 "<Problema>", e termina o processo Servidor com exit code -1.
 *      Caso contrário, dá success S4 "Li Pedido do Cliente";
 *
 * @return Mensagem Elemento com os dados preenchidos.
 */
Mensagem recebePedido() {
    debug("S4 <");

    Mensagem mensagem;

    int msg;

    msg = msgrcv(msgId, &mensagem, sizeof(mensagem.conteudo), 1, 0);
    if (msg == -1)
    {
        error("S4", "Erro ao ler a mensagem");
        exit(-1);
    }
    if (mensagem.conteudo.action != 1)
    {
        error("S4", "Mensagem com action diferente de 1");
        exit(-1);
    }
    success("S4", "Li Pedido do Cliente");

    debug("S4 >");
    return mensagem;
}

/**
 * S5   Cria um processo filho (fork) Servidor Dedicado. Se houver erro, dá error S5 "Fork".
 *      Senão, o processo Servidor Dedicado (filho) continua no passo SD7, 
 *      e o processo Servidor (pai) dá success S5 "Criado Servidor Dedicado com PID <pid Filho>".
 *      Em qualquer dos casos, recomeça o processo no passo S4;
 *
 * @return int PID do processo filho, se for o processo Servidor (pai), 0 se for o processo Servidor Dedicado (filho), ou -1 em caso de erro.
 */
int criaServidorDedicado() {
    debug("S5 <");

    int pidFilho = -1;

    pidFilho = fork();
    if (pidFilho == -1)
        error("S5", "Fork");
    else if (!pidFilho)
        return (0);
    else
        success("S5", "Criado Servidor Dedicado com PID %i", pidFilho);

    debug("S5 >");
    return pidFilho;
}

/**
 * S6  O sinal armado SIGINT serve para o Diretor da Portagem encerrar o Servidor, usando o atalho <CTRL+C>. 
 *      Se receber esse sinal (do utilizador via Shell), o Servidor dá success S6 "Shutdown Servidor", e depois:
 *      S6.1    Envia o sinal SIGHUP a todos os Servidores Dedicados da Lista de Passagens, 
 *              para que concluam o seu processamento imediatamente. Depois, dá success S6.1 "Shutdown Servidores Dedicados";
 *      S6.2    Cria o ficheiro FILE_STATS, escrevendo nele o valor de 3 inteiros (em formato binário), correspondentes a
 *              <contador de passagens Normal>  <contador de passagens Via Verde>  <contador Passagens com Anomalia>
 *              Em caso de erro, dá error S6.2, caso contrário, dá success S6.2 "Estatísticas Guardadas";
 *      S6.3    Dá success S6.3 "Shutdown Servidor completo" e termina o processo Servidor com exit code 0.
 */
void trataSinalSIGINT( int sinalRecebido ) {
    debug("S6 <");

    //S6.1

    success("S6", "Shutdown Servidor");
    for (int i = 0; i != NUM_PASSAGENS; i++)
        if (dadosServidor->lista_passagens[i].tipo_passagem != -1)
            kill(dadosServidor->lista_passagens[i].pid_servidor_dedicado, SIGHUP);
    success("S6.1", "Shutdown Servidores Dedicados");

    //S6.2

    FILE *f;
    f = fopen(FILE_STATS, "w");
    if (access(FILE_STATS, F_OK))
        error("S6.2", "Erro ao criar %s.", FILE_STATS);
    else
    {
        if (fwrite(&dadosServidor->contadores.contadorNormal, sizeof(int), 1, f) < 1
            || fwrite(&dadosServidor->contadores.contadorViaVerde, sizeof(int), 1, f) < 1
            || fwrite(&dadosServidor->contadores.contadorAnomalias, sizeof(int), 1, f) < 1)
            error("S6.2", "Erro ao escrever no ficheiro %s", FILE_STATS);
        else
            success("S6.2", "Estatísticas Guardadas");
    }
    fclose(f);

    //S6.3

    msgctl(msgId, IPC_RMID, NULL);
    semctl(semId, 0, IPC_RMID, 0);
    success("S6.3","Shutdown Servidor completo");
    exit(0);

    debug("S6 >");
}

/**
 * SD7  O novo processo Servidor Dedicado (filho) arma os sinais SIGHUP (ver SD13) e SIGINT (programa para ignorar este sinal).
 *      Depois de armar os sinais, dá success SD7 "Servidor Dedicado Armei sinais";
 *
 * @return int Sucesso
 */
int sd_armaSinais() {
    debug("SD7 <");

    signal(SIGHUP, sd_trataSinalSIGHUP);
    signal(SIGINT, SIG_IGN);
    success("SD7","Servidor Dedicado Armei sinais");

    debug("SD7 >");
    return 0;
}

/**
 * SD8  O Servidor Dedicado deve validar se o pedido que “herdou” do Servidor está corretamente formatado.
 *      Esse pedido inclui uma estrutura Passagem cuja formatação correta tem de validar se:
 *      •   O Tipo de passagem é válido (1 para pedido Normal, ou 2 para Via Verde);
 *      •   A Matrícula e o Lanço não são strings vazias (não é necessário fazer mais validações sobre o seu conteúdo);
 *      •   O pid_cliente é um valor > 0.
 *      Em caso de erro na formatação:
 *      •   Dá error SD8 "<Problema>", e incrementa o contador de anomalias;
 *      •   Se pid_cliente é um valor > 0, envia uma mensagem com action 4 – Pedido Cancelado, para a Message Queue com tipo de mensagem igual ao pid_cliente.
 *          Se o envio tiver erro, dá error SD8 "<Problema>";
 *      •   Ignora o pedido, e termina o processo Servidor Dedicado com exit code -1.
 *      Caso contrário, se não houver erro na formatação, 
 *      dá success SD8 "Chegou novo pedido de passagem do tipo <Normal | Via Verde> solicitado pela viatura com matrícula <matricula> para o Lanço <lanco> e com PID <pid_cliente>";
 *
 * @return int Sucesso
 */
int sd_validaPedido( Mensagem pedido ) {
    debug("SD8 <");

    if ((pedido.conteudo.dados.pedido_cliente.tipo_passagem != 1 && pedido.conteudo.dados.pedido_cliente.tipo_passagem != 2)
        || !strcmp(pedido.conteudo.dados.pedido_cliente.matricula, "") || !strcmp(pedido.conteudo.dados.pedido_cliente.lanco, "")
        || pedido.conteudo.dados.pedido_cliente.pid_cliente < 1)
    {
        error("SD8", "Erro na formatação.");
        sd14(0, -1);
        dadosServidor->contadores.contadorAnomalias++;
        sd14(0, 1);
        if (pedido.conteudo.dados.pedido_cliente.pid_cliente > 0)
        {
            Mensagem msg;
            msg.tipoMensagem = pedido.conteudo.dados.pedido_cliente.pid_cliente;
            msg.conteudo.action = 4;
            if (msgsnd(msgId, &msg, sizeof(msg.conteudo), 0) == -1)
                error("SD8", "Erro ao enviar mensagem");
        }
        exit(-1);
    }
    char *passagem;
    if (pedido.conteudo.dados.pedido_cliente.tipo_passagem == 1)
        passagem = "Normal";
    else
        passagem = "Via Verde";
    success("SD8", "Chegou novo pedido de passagem do tipo %s solicitado pela viatura com matrícula %s para o Lanço %s e com PID %i", passagem, pedido.conteudo.dados.pedido_cliente.matricula, pedido.conteudo.dados.pedido_cliente.lanco, pedido.conteudo.dados.pedido_cliente.pid_cliente);

    debug("SD8 >");
    return 0;
}

/**
 * SD9  Verifica se existe disponibilidade na Lista de Passagens. Se todas as entradas da Lista de Passagens estiverem ocupadas,
 *      dá error SD9 "Lista de Passagens cheia", incrementa o contador de passagens com anomalia, 
 *      manda uma mensagem com action 4 – Pedido Cancelado, para a Message Queue com tipo de mensagem igual ao pid_cliente,
 *      ignora o pedido, e termina o processo Servidor Dedicado com exit code -1.
 *      Caso contrário, preenche uma entrada da lista com os dados deste pedido, incrementa o contador de passagens do tipo de passagem correspondente
 *      e dá success SD9 "Entrada <índice lista> preenchida";
 *
 * @return int Em caso de sucesso, retorna o índice da lista preenchido. Caso contrário retorna -1
 */
int sd_reservaEntradaBD( DadosServidor* dadosServidor, Mensagem pedido ) {
    debug("SD9 <");

    int indiceLista = -1;
    int full = TRUE;

    sd14(1, -1);
    for (int i = 0; i != NUM_PASSAGENS; i++)
    {
        if (dadosServidor->lista_passagens[i].tipo_passagem == -1)
        {
            full = FALSE;
            dadosServidor->lista_passagens[i].tipo_passagem = pedido.conteudo.dados.pedido_cliente.tipo_passagem;
            strcpy(dadosServidor->lista_passagens[i].matricula, pedido.conteudo.dados.pedido_cliente.matricula);
            strcpy(dadosServidor->lista_passagens[i].lanco, pedido.conteudo.dados.pedido_cliente.lanco);
            dadosServidor->lista_passagens[i].pid_cliente = pedido.conteudo.dados.pedido_cliente.pid_cliente;
            dadosServidor->lista_passagens[i].pid_servidor_dedicado =  getpid();
            sd14(0, -1);
            if(pedido.conteudo.dados.pedido_cliente.tipo_passagem==1)
                dadosServidor->contadores.contadorNormal++;
            else
                dadosServidor->contadores.contadorViaVerde++;
            sd14(0, 1);
            indiceLista = i;
            success("SD9", "Entrada %i preenchida", i);
            break ;
        }
    }
    sd14(1, 1);
    if (full)
    {
        error("SD9", "Lista de Passagens cheia");
        sd14(0, -1);
        dadosServidor->contadores.contadorAnomalias++;
        sd14(0, 1);
        Mensagem msg;
        msg.tipoMensagem = pedido.conteudo.dados.pedido_cliente.pid_cliente;
        msg.conteudo.action = 4;
        if (msgsnd(msgId, &msg, sizeof(msg.conteudo), 0) == -1)
            error("SD9", "Erro ao enviar mensagem");
        exit(-1);
    }

    debug("SD9 >");
    return indiceLista;
}

/**
 * "Apaga" uma entrada da Lista de Passagens, colocando tipo_passagem = -1
 *
 * @return int Sucesso
 */
int apagaEntradaBD( DadosServidor* dadosServidor, int indice_lista ) {
    debug("<");

    dadosServidor->lista_passagens[indice_lista].tipo_passagem = -1;

    debug(">");
    return 0;
}


/**
 * SD10 O Servidor Dedicado envia uma mensagem com action 2 – Pedido ACK, para a Message Queue com tipo de mensagem igual ao pid_cliente,
 *      indicando o início do processamento da passagem, e dá success SD10 "Início Passagem <PID Cliente> <PID Servidor Dedicado>";
 *
 * @return int Sucesso
 */
int sd_iniciaProcessamento( Mensagem pedido ) {
    debug("SD10 <");

    Mensagem msg;

    msg.tipoMensagem = pedido.conteudo.dados.pedido_cliente.pid_cliente;
    msg.conteudo.action = 2;
    if (msgsnd(msgId, &msg, sizeof(msg.conteudo), 0) == -1)
        error("SD10", "Erro ao enviar mensagem");
    success("SD10", "Início Passagem %i %i", pedido.conteudo.dados.pedido_cliente.pid_cliente, getpid());

    debug("SD10 >");
    return 0;
}

/**
 * SD11 O Servidor Dedicado calcula um valor aleatório (usando my_rand()) entre os valores MIN_PROCESSAMENTO e MAX_PROCESSAMENTO,
 *      dá success SD11 "<Tempo>", e aguarda esse valor em segundos (sleep);
 *
 * @return int Sucesso
 */
int sd_sleepRandomTime() {
    debug("SD11 <");

    int time;

    time = (my_rand()%(MAX_PROCESSAMENTO - MIN_PROCESSAMENTO)) + MIN_PROCESSAMENTO;
    success("SD11", "%i", time);
    sleep(time);

    debug("SD11 >");
    return 0;
}

/**
 * SD12 O Servidor Dedicado envia uma mensagem com action 3 – Pedido Concluído, para a Message Queue com tipo de mensagem igual ao pid_cliente,
 *      onde também deverá incluir os valores atuais das estatísticas na estrutura contadores_servidor,
 *      indicando o fim do processamento da passagem ao processo <pid_cliente>,
 *      apaga a entrada do Cliente na lista de passagens,
 *      dá success SD12 "Fim Passagem <PID Cliente> <PID Servidor Dedicado>",
 *      e termina o processo Servidor Dedicado;
 *
 * @return int Sucesso
 */
int sd_terminaProcessamento( Mensagem pedido ) {
    debug("SD12 <");

    Mensagem msg;

    msg.tipoMensagem = pedido.conteudo.dados.pedido_cliente.pid_cliente;
    sd14(0, -1);
    msg.conteudo.dados.contadores_servidor = dadosServidor->contadores;
    sd14(0, 1);
    msg.conteudo.action = 3;
    sd14(1, -1);
    for(int i = 0; i != NUM_PASSAGENS; i++)
    {
        if(dadosServidor->lista_passagens[i].pid_cliente == pedido.conteudo.dados.pedido_cliente.pid_cliente)
        {
            apagaEntradaBD(dadosServidor, i);
            break ;
        }
    }
    sd14(1, 1);
    if (msgsnd(msgId, &msg, sizeof(msg.conteudo), 0))
        error("SD12", "Erro ao enviar mensagem");
    success("SD12", "Fim Passagem %i %i", pedido.conteudo.dados.pedido_cliente.pid_cliente, getpid());
    exit(0);
    
    debug("SD12 >");
    return 0;
}

/**
 * SD13 O sinal armado SIGHUP serve para o Servidor indicar que deseja terminar imediatamente o pedido de processamento da passagem.
 *      Se o Servidor Dedicado receber esse sinal, não incrementa o contador de passagens com anomalia, mas manda uma mensagem com action 4 – Pedido Cancelado,
 *      para a Message Queue com tipo de mensagem igual ao pid_cliente, dá success SD13 "Processamento Cancelado", e termina o Servidor Dedicado
 */
void sd_trataSinalSIGHUP(int sinalRecebido) {
    debug("SD13 <");

    Mensagem msg;

    msg.conteudo.action = 4;
    msg.tipoMensagem = mensagem.conteudo.dados.pedido_cliente.pid_cliente;
    if (msgsnd(msgId, &msg, sizeof(msg.conteudo), 0))
        error("SD13", "Erro ao enviar mensagem");
    sd14(1, -1);
    apagaEntradaBD(dadosServidor, indice_lista);
    sd14(1, 1);
    success("SD13", "Processamento Cancelado");
    exit(0);

    debug("SD13 >");
}

/**
 * SD14 Repare que os vários Servidores Dedicados têm todos acesso concorrente à Memória Partilhada.
 *      Acrescente em S3 a criação de um grupo com dois semáforos do tipo MUTEX,
 *      um dedicado à lista de passagens e outro dedicado às estatísticas.
 *      Altere o código do Servidor e do Servidor Dedicado por forma a garantir a exclusão mútua no acesso a cada um
 *      destes dois elementos dos Dados do Servidor, garantindo que o tempo passado em exclusão é sempre o menor possível.
 */
void sd14(int sem_num, int sem_op) 
{
    aux.sem_num = sem_num;
    aux.sem_op = sem_op;
    aux.sem_flg = 0;
    exit_on_error(semop(semId, &aux, 1), "ERROR");
}