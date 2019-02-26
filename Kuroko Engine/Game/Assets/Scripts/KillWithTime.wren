
import "ObjectLinker" for ObjectLinker,
EngineComunicator,
InputComunicator

//For each var you declare, remember to create
//		setters [varname=(v) { __varname = v }]
//		and getters [varname { __varname }]
//The construct method is mandatory, do not erase!
//The import statement at the top og the cript is mandatory, do not erase!
//Be careful not to overwrite the methods declared in Game/ScriptingAPI/ObjectLinker.wren
//[gameObject] is a reserved identifier for the engine, don't use it for your own variables

class KillWithTime is ObjectLinker{

construct new(){}
 
 time_to_destroy { _time_to_destroy}
 time_to_destroy=(v) {_time_to_destroy = v}
 
 starting_time { _starting_time}
 starting_time=(v) { _starting_time = v}
 
 Start() {
  starting_time = EngineComunicator.getTime()
 }

 Update() {
  if(EngineComunicator.getTime() - starting_time > time_to_destroy){
    kill()
  }
 }
}