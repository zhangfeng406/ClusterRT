CFLAGS = -pthread

objects = galois.o jerasure.o reed_sol.o cauchy.o ec.o test.o NetworkUnit.o MetaDataServer.o Common.o Proxy.o BaselineProxy.o ERSProxy.o SRSProxy.o

ElasticEC : $(objects)
	g++  $(CFLAGS) -o ElasticEC $(objects)

galois.o : galois.h
jerasure.o : jerasure.h galois.h
reed_sol.o : jerasure.h galois.h reed_sol.h
cauchy.o : jerasure.h galois.h cauchy.h
ec.o : jerasure.h galois.h reed_sol.h ec.h
Common.o: Common.h
NetworkUnit.o: NetworkUnit.h 
MetaDataServer.o: MetaDataServer.h
Proxy.o: Proxy.h
BaselineProxy.o: BaselineProxy.h
ERSProxy.o: ERSProxy.h
SRSProxy.o: SRSProxy.h
test.o : ec.h 

.PHONY : clean
clean :
	rm -f $(objects) ElasticEC
