WrapperDll
==========

Il WrapperDll è un programma C++ client/server per remotizzare l'esecuzione di funzioni presenti in una DLL sul server. Utilizza boost e protobuf. Il client si collegherà al server e chiederà di aprire una DLL. Otterrà un handle che dovrà essere utilizzato per eseguire le funzioni in essa contenute. Al termine dovrà essere rilasciata la DLL impiegando l'opportuno comando di chiusura libreria. E' possibile aprire più DLL contemporaneamente. 

Sul canale socket transiteranno quindi le richieste dal client verso il server e ritorneranno i dati dell'elaborazione. Le funzioni verranno TUTTE eseguite sul server che potrà quindi essere su un'altra macchina. 

I client potranno essere scritti in Java o C++, ma anche in python.

Utilizzando questa architettura una macchina Unix potrà eseguire delle funzioni presenti su di una DLL che viene eseguita sul server installato su di una macchina windows.

Se installato sulla stessa le funzioni delle DLL verranno eseguite in un processo separato, questo consentirà di evitare il crash del client nel caso di malfunzionamenti nella DLL.

Librerie utilizzate:  boost	 release 1.49 protobuf release 2.4.1 glog	 release 0.3.2 gflags   release 2.0
