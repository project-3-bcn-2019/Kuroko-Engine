
import "ObjectLinker" for ObjectLinker, EngineComunicator, InputComunicator

//For each var you declare, remember to create
//		setters [varname=(v) { __varname = v }]
//		and getters [varname { __varname }]
//The construct method is mandatory, do not erase!
//The import statement at the top og the cript is mandatory, do not erase!
//Be careful not to overwrite the methods declared in Game/ScriptingAPI/ObjectLinker.wren
//[gameObject] is a reserved identifier for the engine, don't use it for your own variables

class KillOnSpace is ObjectLinker{

construct new(){}

 Start() {}

 Update() {
   if(InputComunicator.getKey(InputComunicator.SPACE, InputComunicator.KEY_DOWN)){
     kill()
   }
 }

}