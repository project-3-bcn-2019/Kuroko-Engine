
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

class enemy_caller is ObjectLinker{

construct new(){}

 Start() {
	
	var game_objects = EngineComunicator.FindGameObjectsByTag("enemy")

	for(i in 0...game_objects.count){
		EngineComunicator.consoleOutput("Enemy %(i) spotted at: %(game_objects[i].getPos("global").x), %(game_objects[i].getPos("global").y), %(game_objects[i].getPos("global").z)")
	}

 }
}