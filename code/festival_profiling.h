

#define ProfileCycleFrameCount 120

string_list ProfileNames = StringList();
double ProfileResultFrames[10][ProfileCycleFrameCount];
int CurrentProfileIndex = 0;
int CurrentProfileFrame = 0;


#if 0
if(CurrentProfileFrame >= ProfileCycleFrameCount)
CurrentProfileFrame = 0;
CurrentProfileIndex = 0;
for(int i = 0; i < ProfileNames.Count; i++)
{
    FreeString(ProfileNames[i]);
}
ListFree(&ProfileNames);
ProfileNames = StringList();
#endif

#define DefineProfile(Name) {\
ListAdd(&ProfileNames, String("%s", #Name));\
}\

#define StartProfiling() { \
if(CurrentProfileFrame >= ProfileCycleFrameCount)\
CurrentProfileFrame = 0;\
CurrentProfileIndex = 0;\
CurrentProfileFrame++;\
}
#define StartProfile(Name) double Name##ProfileTimeStart = GetTime();
#define EndProfile(Name) {\
double Name##ProfileTimeEnd = GetTime();\
double Name##TotalMS = (Name##ProfileTimeEnd - Name##ProfileTimeStart) * 1000;\
/*printf("%s: %lfms\n", #Name, Name##TotalMS);*/\
ProfileResultFrames[CurrentProfileIndex][CurrentProfileFrame] = Name##TotalMS;\
CurrentProfileIndex++;\
}
#define PrintProfiles() {\
if(CurrentProfileFrame % 3 == 0)\
{\
for(int i = 0; i < ProfileNames.Count; i++)\
{\
double Total = 0;\
for(int a = 0; a < ProfileCycleFrameCount; a++)\
{\
Total += ProfileResultFrames[i][a];\
}\
Print("%S: %f", ProfileNames[i], Total/ProfileCycleFrameCount);\
}\
printf("\n");\
}\
}
