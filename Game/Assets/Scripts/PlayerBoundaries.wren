
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

class PlayerBoundaries is ObjectLinker{

construct new(){}

x_positive {__x_positive}
x_positive=(v){__x_positive = v}

x_negative {__x_negative}
x_negative=(v){__x_negative = v}

z_positive {__z_positive}
z_positive=(v){__z_positive = v}

z_negative {__z_negative}
z_negative=(v){__z_negative = v}

 Start() {}

 Update() {
  var pos_x = getPosX("global")
  var pos_z = getPosZ("global")
EngineComunicator.consoleOutput("%(pos_x)")
  if(pos_x > x_positive){
   setPos(x_positive,getPosY("global"),pos_z)
  }

  if(pos_x < x_negative){
   setPos(x_negative,getPosY("global"),pos_z)
  }

  if(pos_z > z_positive){
   setPos(pos_x,getPosY("global"),z_positive)
  }

  if(pos_z < z_negative){
   setPos(pos_x,getPosY("global"),z_negative)
  }



 }
}