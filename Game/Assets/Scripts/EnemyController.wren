
import "ObjectLinker" for ObjectLinker,
EngineComunicator,
InputComunicator,
ComponentType,
Time,
Vec3

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
ShowDebuLogs=(v) { _show_debug_logs = v }

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
MoveSpeed=(v) { _move_speed = v }

Damage {_damage}
Damage=(v) { _damage = v }

AttackSpeed {_attack_speed}
AttackSpeed=(v) { _attack_speed = v }

AttackRange {_attack_range}
AttackRange=(v) { _attack_range = v }

Health {_health}
Health=(v) { _health = v }

HitStun {_hit_stun}
HitStun=(v) { _hit_stun = v }

Sight {_sight}
Sight=(v) { _sight = v }

Range {_range}
Range=(v) { _range = v }

//Other variables
AlitaSeen {_alita_seen}
AlitaInRange {_alita_in_range}
Damaged{_damaged}
Target{_target}

State = (new_state) {
        if (_enemy_state)  _enemy_state.EndState() //the first time player_state is null
        _enemy_state = new_state   
        _enemy_state.BeginState()
    }

 Start() {

     _enemy_state

    _idle_state = IdleState.new(this)
    _chase_state = ChaseState.new(this)
    _attack_state = AttackState.new(this)
    _hit_state = HitState.new(this)
    _dead_state = DeadState.new(this)

    this.State = _idle_state
 }

 Update() {
     lookForAlita()

     _enemy_state.HandleInput()
     _enemy_state.Update()
 }

 lookForAlita(){
     if(_target == null){
         _target = EngineComunicator.FindGameObjectsByTag("Alita")[0]
     }
    var own_pos = getPos("global")
    var target_pos = _target.getPos("global")
     var distance = Vec3.new(own_pos.x-target_pos.x,own_pos.y-target_pos.y,own_pos.z-target_pos.z)
     distance = distance.magnitude

     if(distance < _sight){
         _alita_seen = true

     } else{
         _alita_seen = false
     }

     if(distance < _attack_range){
         _alita_in_range = true
     } else {
         _alita_in_range = false
     }
 }
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

        HandleInput(){


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
                  EngineComunicator.consoleOutput("SHIT CREATED")
        super.BeginState()
    }

    HandleInput(){
        //If alita is detected
             EngineComunicator.consoleOutput("update enemy")
        if(_enemy.AlitaSeen){
            _enemy.State = _enemy.ChaseState
                        EngineComunicator.consoleOutput("change to chase")
        }
    }

    Update(){
        super.Update()
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
            if(_enemy.AlitaInRange){
                _enemy.State = _enemy.AttackState
            } else {
                _enemy.State = _enemy.ChaseState
            }
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
             if(_enemy.AlitaInRange){
                _enemy.State = _enemy.AttackState
            } else {
                _enemy.State = _enemy.ChaseState
            }
            //if alita is in range, attack again.
            //if not, go to chase alita
        }
    }

}

class DeadState is EnemyState{
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

