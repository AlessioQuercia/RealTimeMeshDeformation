##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=work06a_win
ConfigurationName      :=Debug
WorkspacePath          :=C:/Users/Alessio/Documents/GitHub/RGP_LAB/1819lab7
ProjectPath            :=C:/Users/Alessio/Documents/GitHub/RGP_LAB/1819lab7/work/work06
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Alessio
Date                   :=05/06/2019
CodeLitePath           :="C:/Program Files/CodeLite"
LinkerName             :=C:/MinGW/bin/g++.exe
SharedObjectLinkerName :=C:/MinGW/bin/g++.exe -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="work06a_win.txt"
PCHCompileFlags        :=
MakeDirCommand         :=C:/MinGW/bin/makedir.exe
RcCmpOptions           := 
RcCompilerName         :=C:/MinGW/bin/windres.exe
LinkOptions            :=  -static-libgcc -static-libstdc++
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)../../include 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)glfw3 $(LibrarySwitch)assimp 
ArLibs                 :=  "glfw3" "assimp" 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)../../libs/win 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := C:/MinGW/bin/ar.exe rcu
CXX      := C:/MinGW/bin/g++.exe
CC       := C:/MinGW/bin/gcc.exe
CXXFLAGS :=  -g -O0 -Wall -std=c++0x $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := C:/MinGW/bin/as.exe


##
## User defined environment variables
##
CodeLiteDir:=C:\Program Files\CodeLite
Objects0=$(IntermediateDirectory)/work06a.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_up_include_glad_glad.c$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

PostBuild:
	@echo Executing Post Build commands ...
	copy ..\..\libs\win\*.dll .\Debug
	copy *.vert Debug
	copy *.frag Debug
	@echo Done

MakeIntermediateDirs:
	@$(MakeDirCommand) "./Debug"


$(IntermediateDirectory)/.d:
	@$(MakeDirCommand) "./Debug"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/work06a.cpp$(ObjectSuffix): work06a.cpp $(IntermediateDirectory)/work06a.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "C:/Users/Alessio/Documents/GitHub/RGP_LAB/1819lab7/work/work06/work06a.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/work06a.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/work06a.cpp$(DependSuffix): work06a.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/work06a.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/work06a.cpp$(DependSuffix) -MM work06a.cpp

$(IntermediateDirectory)/work06a.cpp$(PreprocessSuffix): work06a.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/work06a.cpp$(PreprocessSuffix) work06a.cpp

$(IntermediateDirectory)/up_up_include_glad_glad.c$(ObjectSuffix): ../../include/glad/glad.c $(IntermediateDirectory)/up_up_include_glad_glad.c$(DependSuffix)
	$(CC) $(SourceSwitch) "C:/Users/Alessio/Documents/GitHub/RGP_LAB/1819lab7/include/glad/glad.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_up_include_glad_glad.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_up_include_glad_glad.c$(DependSuffix): ../../include/glad/glad.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_up_include_glad_glad.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_up_include_glad_glad.c$(DependSuffix) -MM ../../include/glad/glad.c

$(IntermediateDirectory)/up_up_include_glad_glad.c$(PreprocessSuffix): ../../include/glad/glad.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_up_include_glad_glad.c$(PreprocessSuffix) ../../include/glad/glad.c


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


