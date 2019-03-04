
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

class vec3_test is ObjectLinker{

construct new(){}

 Start() {
   _vec3 = Vec3.new(3,3,3)
    EngineComunicator.consoleOutput("Magnitude %(_vec3.magnitude)")
	EngineComunicator.consoleOutput("x = %(_vec3.x), y = %(_vec3.y), z = %(_vec3.z)")

   _vec3_other = _vec3.normalized
   EngineComunicator.consoleOutput("x = %(_vec3_other.x), y = %(_vec3_other.y), z = %(_vec3_other.z)")

   _vec3.normalize()
   EngineComunicator.consoleOutput("x = %(_vec3.x), y = %(_vec3.y), z = %(_vec3.z)")
 }
}