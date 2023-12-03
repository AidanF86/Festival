
string_list ProfileResultStrings = StringList();

#define StartProfiling() { \
for(int i = 0; i < ProfileResultStrings.Count; i++)\
{\
FreeString(ProfileResultStrings[i]);\
}\
for(int i = 0; i < ProfileResultStrings.Count; i++)\
{\
/*ListRemoveAt(&ProfileResultStrings, 0);*/\
}\
ListFree(&ProfileResultStrings);\
ProfileResultStrings = StringList();\
}
#define StartProfile(Name) double Name##ProfileTimeStart = GetTime();
#define EndProfile(Name) {\
double Name##ProfileTimeEnd = GetTime();\
double Name##TotalMS = (Name##ProfileTimeEnd - Name##ProfileTimeStart) * 1000;\
/*printf("%s: %lfms\n", #Name, Name##TotalMS);*/\
ListAdd(&ProfileResultStrings, String("%s: %f", #Name, Name##TotalMS));\
}
#define PrintProfiles() {\
for(int i = 0; i < ProfileResultStrings.Count; i++)\
{\
Print(ProfileResultStrings[i]);\
}\
printf("\n");\
}
