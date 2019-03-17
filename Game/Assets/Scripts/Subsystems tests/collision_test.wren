
import "ObjectLinker" for ObjectLinker,
EngineComunicator,
InputComunicator,
ComponentType,
Vec3

import "Physics" for ComponentPhysics

//For each var you declare, remember to create
//		setters [varname=(v) { __varname = v }]
//		and getters [varname { __varname }]
//The construct method is mandatory, do not erase!
//The import statement at the top og the cript is mandatory, do not erase!
//Be careful not to overwrite the methods declared in Game/ScriptingAPI/ObjectLinker.wren
//[gameObject] is a reserved identifier for the engine, don't use it for your own variables

class collision_test is ObjectLinker{

construct new(){}

 Start(){
 _physics = getComponent(ComponentType.PHYSICS)

 var speed = Vec3.new(3,0,0)
 _physics.setSpeed(speed)
 }
 Update() {
  var collisions_go = getCollisions()
  EngineComunicator.consoleOutput("%(collisions_go.count)")
 }
}