
import "ObjectLinker" for ObjectLinker,
EngineComunicator,
InputComunicator,
Vec3

//For each var you declare, remember to create
//		setters [varname=(v) { __varname = v }]
//		and getters [varname { __varname }]
//The construct method is mandatory, do not erase!
//The import statement at the top og the cript is mandatory, do not erase!
//Be careful not to overwrite the methods declared in Game/ScriptingAPI/ObjectLinker.wren
//[gameObject] is a reserved identifier for the engine, don't use it for your own variables

class instantiate_test is ObjectLinker{

construct new(){}

 Start() {
      _cube = instantiate("Cube", Vec3.new(10,10,0), Vec3.new(0,0,0))
 }

 Update(){
   if(InputComunicator.getKey(InputComunicator.SPACE, InputComunicator.KEY_REPEAT)){
     _cube.modPos(-10,0,0)
   }
 }

}