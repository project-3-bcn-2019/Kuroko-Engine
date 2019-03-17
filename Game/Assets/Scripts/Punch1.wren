
import "ObjectLinker" for ObjectLinker,
EngineComunicator,
InputComunicator,
Vec3,
Time,
ComponentType,
Math

//For each var you declare, remember to create
//		setters [varname=(v) { __varname = v }]
//		and getters [varname { __varname }]
//The construct method is mandatory, do not erase!
//The import statement at the top og the cript is mandatory, do not erase!
//Be careful not to overwrite the methods declared in Game/ScriptingAPI/ObjectLinker.wren
//[gameObject] is a reserved identifier for the engine, don't use it for your own variables

class Punch1 is ObjectLinker{

    AttackSpeed {_attack_speed}
    AttackSpeed=(new_attack_speed) { _attack_speed = new_attack_speed }

    Damage {_damage}
    Damage=(new_damage) { _damage = new_damage }

construct new(){}

 Start() {
    _total_time_alive = 1000
    _current_time_alive = 0
 }

 Update() {
    _current_time_alive = _current_time_alive + Time.C_GetDeltaTime()

    if (_current_time_alive >= _total_time_alive) {
       kill()
    }
 }
}