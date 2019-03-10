
import "ObjectLinker" for ObjectLinker,
EngineComunicator,
InputComunicator,
ComponentType,
Time

//For each var you declare, remember to create
//		setters [varname=(v) { __varname = v }]
//		and getters [varname { __varname }]
//The construct method is mandatory, do not erase!
//The import statement at the top og the cript is mandatory, do not erase!
//Be careful not to overwrite the methods declared in Game/ScriptingAPI/ObjectLinker.wren
//[gameObject] is a reserved identifier for the engine, don't use it for your own variables

class EnemyController is ObjectLinker{

construct new(){}

ShowDebuLogs{_show_debug_logs}

//States
IdleState{_idle_state}
ChaseState{_chase_state}
AttackState{_attack_state}
HitState{_hit_state}
DeadState{_dead_state}
 
 //Components
ComponentAnimation {_component_animation}
ComponentAudioSource {_component_audio_source}

//Stats
MoveSpeed {_move_speed}
Damage {_damage}
AttackSpeed {_attack_speed}
AttackRange {_attack_range}
Health {_health}
HitStun {_hit_stun}

//Other variables
AlitaSeen {_alita_seen}
AlitaInRange {_alita_in_range}
Damaged{_damaged}

State = (new_state) {
        if (_player_state)  _player_state.EndState() //the first time player_state is null
        _player_state = new_state   
        _player_state.BeginState()
    }

 Start() {}

 Update() {}
}
 //State Machine structure taken from Pol Ferrando... 
 //...and his Alita state machine for the sake of clarity
class EnemyState {
    TotalDuration {_total_duration}
    CurrentTime {_current_time_in}

    construct new(enemy){
        _enemy = enemy
    }

    construct new(enemy,duration){
        _enemy = enemy
        _total_duration = duration
    }


    BeginState(){
        _current_time_in = 0
    }

    Update(){
        if(_enemy.Damaged){
            _enemy.State = _enemy.HitState
        }
    }

    EndState(){

    }

    UpdateCurrentTime(){
        _current_time_in = _current_time_in + Time.C_GetDeltaTime() 
    }

    IsStateFinished() {
        var ret = false
        if (_current_time_in >= _total_duration) ret = true
        return ret
    }

}

class IdleState is  EnemyState{
    construct new(enemy){
        super(enemy)
    }

    BeginState(){
        _enemy.ComponentAnimation.setAnimation("IdleAnimation")
        _enemy.ComponentAnimation.Reset()
        super.BeginState()
    }

    HandleInput(){
        //If alita is detected
        if(_enemy.AlitaSeen){
            _enemy.State = _enemy.ChaseState
        }
    }

    Update(){
        super.Update()
        //The state should look for alita here???
        if(_enemy.ShowDebuLogs){
            EngineComunicator.consoleOutput("Current state: Idle")
        }
    }
}

class ChaseState is EnemyState{
    construct new(enemy){
        super(enemy)
        _enemy = enemy
    }

    BeginState(){
        _enemy.ComponentAnimation.setAnimation("RunningAnimation")
        _enemy.ComponentAnimation.Reset()
        _enemy.ComponentAnimation.Play()

    }

    HandleInput(){
        if(_enemy.AlitaInRange){
            _enemy.State = _enemy.AttackState
        }
    }

    Update(){
        //check the distance here
         EngineComunicator.consoleOutput("Current state: Chasing")
    }
}

class AttackState is EnemyState{
    construct new(enemy){
        super(enemy,enemy.AttackSpeed)
        _enemy = enemy
    }

    BeginState(){
        _enemy.ComponentAnimation.setAnimation("AttackAnimation")
        _enemy.ComponentAnimation.Reset()
        _enemy.ComponentAnimation.Play()

    }

    EndState(){

    }

    HandleInput() {

    }

    Update() {
        if(super.IsStateFinished()){
       //had to have something
        //if alita is still in range, attack again.
        //if not, go to chase alita
        }
    }
    
}

class HitState is EnemyState{
    construct new(enemy){
        super(enemy,enemy.HitStun)
        _enemy = enemy
    }

    BeginState(){
        _enemy.ComponentAnimation.setAnimation("HitAnimation")
        _enemy.ComponentAnimation.Reset()
        _enemy.ComponentAnimation.Play()
    }

    EndState(){

    }

    HandleInput() {

    }

    Update(){
        super.Update()
        if(super.IsStateFinished()){
            //if alita is in range, attack again.
            //if not, go to chase alita
        }
    }

}

class DeathState is EnemyState{
    construct new(enemy){
        super(enemy)
        _enemy = enemy
    }

    BeginState(){
        _enemy.ComponentAnimation.setAnimation("DeathAnimation")
        _enemy.ComponentAnimation.Reset()
        _enemy.ComponentAnimation.Play()
    }

    EndState(){

    }

    HandleInput() {

    }

    Update(){
        super.Update()
        if(super.IsStateFinished()){
            //delet this
        }
    }

}

