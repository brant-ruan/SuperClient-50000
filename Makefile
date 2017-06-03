object = mMain.o mArg.o mLog.o mConfig.o mClient.o mSocket.o mProtocol.o

ts: $(object)
	gcc -o ts $(object)

mMain.o: mMain.c

mArg.o: mArg.c

mLog.o: mLog.c

mConfig.o: mConfig.c

mClient.o: mClient.c

mSocket.o: mSocket.c

mProtocol.o: mProtocol.c


.PHONY:
clean:
	-rm $(object)
