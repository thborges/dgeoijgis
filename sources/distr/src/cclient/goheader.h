
#ifndef GO_HEADER_H
#define GO_HEADER_H

//from src/cclient/join.go
extern void NewJoin(const char *query_name, unsigned char);
extern void EndJoin(unsigned char);
extern void StartJoinMsg(char *, ushort, ushort, int, unsigned char);
extern void AddRToJoinMsg(char *, ushort, ushort, int);
extern void EndJoinMsg(int);
//from src/cclient/load.go
extern void NewDataset(const char *, dataset_histogram_persist*, int, int);
extern void EndDataset();

extern void CleanIntermediate();
extern int ActivePeersCount();

#endif

