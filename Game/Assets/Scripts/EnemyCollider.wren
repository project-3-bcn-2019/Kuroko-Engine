
import "ObjectLinker" for ObjectLinker,
EngineComunicator,
InputComunicator,
Time

//For each var you declare, remember to create
//		setters [varname=(v) { __varname = v }]
//		and getters [varname { __varname }]
//The construct method is mandatory, do not erase!
//The import statement at the top og the cript is mandatory, do not erase!
//Be careful not to overwrite the methods declared in Game/ScriptingAPI/ObjectLinker.wren
//[gameObject] is a reserved identifier for the engine, don't use it for your own variables

class EnemyCollider is ObjectLinker{

Damage{_damage}
DamageMultiplier{_damage_multiplier}
ActiveMS{_active_ms}

construct new(){}

 Start() {
     _current_time = 0.0
 }

 Update() {

     var collisions = getCollisions()
     for(i in 0...collisions.count){
         var Alita = collisions[i].getScript("PlayerController")
         if(Alita){
             Alita.dealDamage(_damage,_damage_multiplier)
         }
     }

    _current_time = _current_time + Time.C_GetDeltaTime()
    if(_current_time >= _active_ms){
        kill()
    }

 }
}