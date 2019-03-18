
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

class PlayerController is ObjectLinker{

    // Getters
    ShowDebugLogs{_show_debug_logs}

    // Below this the values are not meant to be changed in the inspector (also there is no setter so they can't)
    MovingState {_moving_state}
    IdleState {_idle_state}
    DashState {_dash_state}
    Punch1 {_punch1_state}
    Punch2 {_punch2_state}
    Kick1 {_kick1_state}
    Kick2 {_kick2_state}
    MoveDirection {_move_direction}
    OldMoveDirection {_old_move_direction}

    ComponentAudioSource {_component_audio_source}
    ComponentAnimatior {_component_animator}


    PunchButton {_punch_button}
    KickButton {_kick_button}
    DashButton {_dash_button}

    //Stats
    MoveSpeed {_move_speed}
    MoveSpeed=(v) { _move_speed = v }

    Damage {_damage}
    Damage=(v) { _damage = v }

    Health {_health}    
    Health=(v) { _health = v }

    Defense {_defense}
    Defense=(v) { _defense = v }

    HitStun {_hit_stun}
    HitStun=(v) { _hit_stun = v }

    Sight {_sight}
    Sight=(v) { _sight = v }

    //Other variables
    Damaged{_damaged}
    Dead{_dead}


    // Setters
    State = (new_state) {
        if (_player_state)  _player_state.EndState() //the first time player_state is null
        _player_state = new_state   
        _player_state.BeginState()
    }

    ShowDebugLogs = (value) {_show_debug_logs = value} 


    construct new(){}

    Start() {
        _show_debug_logs = false
        _player_state
        _move_direction = Vec3.zero()
        _old_move_direction = Vec3.zero()

        _punch_button = InputComunicator.C_X
        _kick_button = InputComunicator.C_Y
        _dash_button = InputComunicator.C_A

        //Components
        _component_audio_source = getComponent(ComponentType.AUDIO_SOURCE)
        _component_animator = getComponent(ComponentType.ANIMATOR)
        _component_audio_source.setSound("Footsteps") //This should not be here -Pol

        //Initialize all the states
        _idle_state = IdleState.new(this)

        //The arguments for a bsic attack are (player,type,tier,animation_name,sound_name,total_duration)
        _punch1_state = BasicAttackState.new(this,"punch",1,"punch1","Punch","P1Col",700,500)
        _punch2_state = BasicAttackState.new(this,"punch",2,"punch2","Punch","P1Col",1000,500)
        _kick1_state = BasicAttackState.new(this,"kick",1,"kick1","Punch","P1Col",700,500)
        _kick2_state = BasicAttackState.new(this,"kick",2,"kick2","Punch","P1Col",1000,500)

        _moving_state = MovingState.new(this)
        _dash_state = DashState.new(this,500)


        //Movement
        _move_script = getScript("EntityMove")

        //this "this" I believe that should not be necesary but if removed, script won't compile    -p
        this.State = _idle_state //Reminder that "State" is a setter method
    }

    Update() {
        this.CalculateDirection()
        _player_state.HandleInput()
        _player_state.Update()
    }

    CalculateDirection() {
        //positive y means up and positive x means right
        _move_direction.y = -InputComunicator.getAxisNormalized(-1,InputComunicator.L_AXIS_Y)
        _move_direction.x = -InputComunicator.getAxisNormalized(-1,InputComunicator.L_AXIS_X)

        if(InputComunicator.getKey(InputComunicator.UP, InputComunicator.KEY_REPEAT)){
	       _move_direction.y = 1
        }
	 	
        if(InputComunicator.getKey(InputComunicator.DOWN, InputComunicator.KEY_REPEAT)){
            _move_direction.y = -1
        }

        if(InputComunicator.getKey(InputComunicator.LEFT, InputComunicator.KEY_REPEAT)){
            _move_direction.x = 1
        }
                
        if(InputComunicator.getKey(InputComunicator.RIGHT, InputComunicator.KEY_REPEAT)){
        _move_direction.x = -1
        }

        if(_move_direction.y < 0.2 && _move_direction.y > -0.2)   _move_direction.y = 0.0
        if(_move_direction.x < 0.2 && _move_direction.x > -0.2)   _move_direction.x = 0.0
    }

    SetSpeed(x,y,z) {

        _move_script.SetSpeed(x,y,z)
        //super.modPos(x,y,z)
    } 
 
    AccelerateTo(x,y,z) {
        _move_script.AccelerateTo(x,y,z)
        //super.modPos(x,y,z)
    } 

    SetDeceleration(num){
        _move_script.deceleration = num

    }
    rotate(x,y,z) {
         //_move_script.RotateTo(x,y,z)
    }

    dealDamage(damage,multiplier){
        if(!_damaged){
        _health = _health - ((damage*multiplier) - (_defense/2))
        _damaged = true
        EngineComunicator.consoleOutput("Dealt%(damage)")
        }
    }

}

class State {
    TotalDuration {_total_duration}
    CurrentTime {_current_time_in}

    construct new(player) {
        _player = player
    }

    construct new(player,total_duration) {
        _player = player
        _total_duration = total_duration
    }

    //Called one time when switching to the state
    BeginState() {
        _current_time_in = 0

        if (_player.ShowDebugLogs) EngineComunicator.consoleOutput("new state began")
    }

    EndState() {

    }

    //Here are all the functions that all the states will do in update, remember to call super.Update() -p
    Update() {
        this.UpdateCurrentTime()
    }

    UpdateCurrentTime() {
        _current_time_in = _current_time_in + Time.C_GetDeltaTime() // += does not work -p

        if (_player.ShowDebugLogs)  EngineComunicator.consoleOutput("Current time in this state:%(_current_time_in)")
    }

    IsStateFinished() {
        var ret = false
        if (_current_time_in >= _total_duration) ret = true
        return ret
    }
}

class IdleState is State {
    construct new() {
    }

    construct new(player) {
        _player = player
        super(player)
    }

    BeginState() {
        _player.ComponentAnimatior.setBool("idle",true)
        super.BeginState()
    }

    EndState() {
        _player.ComponentAnimatior.setBool("idle",false)        
    }

    HandleInput() {
        // If l-stick is not still switch to moving
        if(_player.MoveDirection.x != 0.0 || _player.MoveDirection.y != 0.0) _player.State = _player.MovingState
        // If X prassed switch to punch
        if (InputComunicator.getButton(-1,_player.PunchButton, InputComunicator.KEY_DOWN)) _player.State = _player.Punch1
        if (InputComunicator.getKey(InputComunicator.J, InputComunicator.KEY_DOWN)) _player.State = _player.Punch1
        // 
        if (InputComunicator.getButton(-1,_player.KickButton, InputComunicator.KEY_DOWN)) _player.State = _player.Kick1
        if (InputComunicator.getKey(InputComunicator.K, InputComunicator.KEY_DOWN)) _player.State = _player.Kick1
    }

    Update() {
        super.Update()
        if (_player.ShowDebugLogs){  
            EngineComunicator.consoleOutput("Current state: Idle")
        }
    }

}

class MovingState is State {

    construct new(player) {
        super(player)
        _player = player
    }

    BeginState() {
        super.BeginState()
        _player.ComponentAnimatior.setBool("moving",true)
    }

    EndState() {
        _player.ComponentAnimatior.setBool("moving",false)        
    }

    HandleInput() {
        if(_player.MoveDirection.x == 0.0 && _player.MoveDirection.y == 0.0) _player.State = _player.IdleState
        // If A prassed switch to dash
        if (InputComunicator.getButton(-1,_player.DashButton, InputComunicator.KEY_DOWN)) _player.State = _player.DashState
        if (InputComunicator.getKey(InputComunicator.SPACE, InputComunicator.KEY_DOWN)) _player.State = _player.DashState
        // If X prassed switch to dash
        if (InputComunicator.getButton(-1,_player.PunchButton, InputComunicator.KEY_DOWN)) _player.State = _player.Punch1
        if (InputComunicator.getKey(InputComunicator.J, InputComunicator.KEY_DOWN)) _player.State = _player.Punch1

        // If Y prassed switch to dash
        if (InputComunicator.getButton(-1,_player.KickButton, InputComunicator.KEY_DOWN)) _player.State = _player.Kick1
        if (InputComunicator.getKey(InputComunicator.K, InputComunicator.KEY_DOWN)) _player.State = _player.Kick1
    }
    
    Update() {
        super.Update()

        //Obsolete 
        var movement = Vec3.new(_player.MoveDirection.x*_player.MoveSpeed,0,_player.MoveDirection.y*_player.MoveSpeed)

        _player.AccelerateTo(movement.x,movement.y,movement.z)

        //var angle = Math.C_angleBetween(_player.OldMoveDirection.x,_player.OldMoveDirection.y,_player.OldMoveDirection.z,_player.MoveDirection.x,_player.MoveDirection.y,_player.MoveDirection.z)
       // _player.rotate(_player.MoveDirection.x,_player.MoveDirection.y,_player.MoveDirection.z)
        //_player.rotate(_player.MoveDirection.x, _player.MoveDirection.y, _player.MoveDirection.z)

        _player.OldMoveDirection.x = _player.MoveDirection.x
        _player.OldMoveDirection.y = _player.MoveDirection.y
        _player.OldMoveDirection.z = _player.MoveDirection.z

        if (_player.ShowDebugLogs){  
            EngineComunicator.consoleOutput("Current state: Moving")
            EngineComunicator.consoleOutput("direction.x =%(_player.MoveDirection.x)")
            EngineComunicator.consoleOutput("direction.y =%(_player.MoveDirection.y)")
        }
    }
}

class DashState is State {
    construct new(player) {
        super(player)
        _player = player
    }

    construct new(player,total_duration) {
        _player = player
        super(player,total_duration)
    }

    BeginState() {
        super.BeginState()

        _dash_direction = Vec3.new(_player.MoveDirection.x,_player.MoveDirection.y,_player.MoveDirection.z)
        _dash_speed = 600
        _player.SetDeceleration(0.05)

    }

    HandleInput() {

    }
    
    Update() {
        super.Update() 

        var movement = Vec3.new(_dash_direction.x*_dash_speed,0,_dash_direction.y*_dash_speed)

        _player.SetSpeed(movement.x,movement.y,movement.z)

        if (super.IsStateFinished()) _player.State = _player.IdleState

        if (_player.ShowDebugLogs){  
            EngineComunicator.consoleOutput("direction.x =%(_dash_direction.x)")
            EngineComunicator.consoleOutput("direction.y =%(_dash_direction.y)")
            EngineComunicator.consoleOutput("Current state: Dash")
        }
    }

    EndState(){
        _player.SetDeceleration(0.01)
    }

}

class BasicAttackState is State {
    construct new(player) {
        super(player)
        _player = player
    }
    //The animation name is the name of the variable that the graph uses
    //when the windup duration finishes the collider will be created, make sure its lower than the total_duration (Its obvious)
    construct new(player,type,tier,animation_name,sound_name,prefav_collider_name,total_duration,windup_duration) {
        _player = player
        _type = type
        _tier = tier
        _animation_name = animation_name
        _sound_name = sound_name
        _prefav_collider_name = prefav_collider_name
        
        _windup_duration = windup_duration

        
        if (windup_duration > total_duration) {
            EngineComunicator.consoleOutput("Error, the windup duration must not be higher than the total duration")
        }


        super(player,total_duration)
    }

    BeginState() {
        super.BeginState()

        _next_state = _player.IdleState
        _on_contact_done = false

        _player.ComponentAnimatior.setBool(_animation_name,true)

        _player.ComponentAudioSource.setSound(_sound_name)
        _player.ComponentAudioSource.Play()
    }

    EndState() {
        _player.ComponentAnimatior.setBool(_animation_name,false)        
    }

    HandleInput() {
        _margin_to_chain_attack = 200 //totally invented 


        if (super.CurrentTime > (super.TotalDuration - _margin_to_chain_attack)) {
            if (_tier == 1) {
                if (InputComunicator.getButton(-1,_player.PunchButton, InputComunicator.KEY_DOWN)) _next_state = _player.Punch2
                if (InputComunicator.getButton(-1,_player.KickButton, InputComunicator.KEY_DOWN)) _next_state = _player.Kick2
            }
        }
        
    }

    //This is when the attak will instanciate the collider 
    OnContactFrames() {
        EngineComunicator.consoleOutput("Create collider")

        var forward = _player.getForward()

        var multiplier = 5
        var col_height = 10
        var x_offset = forward.x * multiplier
        var z_offset = forward.z * multiplier

        _player.instantiate(_prefav_collider_name, Vec3.new(x_offset,col_height,z_offset), Vec3.new(0,0,0))
    }
    
    GoToNextState() {
        _player.State = _next_state
    }

    Update() {
        super.Update() 

        if (super.CurrentTime >= _windup_duration && _on_contact_done == false){
            this.OnContactFrames()
            _on_contact_done = true
        } 

        if (super.IsStateFinished()) this.GoToNextState()

        if (_player.ShowDebugLogs) EngineComunicator.consoleOutput("Current state: %(_type) %(_tier)")
    }
}

class SpecialAttackState is State {
    construct new(player) {
        super(player)
    }
}

class HitState is State{
    construct new(player){
        super(player,player.HitStun)
        _player = player
    }

    BeginState(){
        _next_state = _player.IdleState
        // _enemy.ComponentAnimation.setAnimation("HitAnimation")
        // _enemy.ComponentAnimation.Reset()
        // _enemy.ComponentAnimation.Play()
        super.BeginState()
    }

    EndState(){

    }

    HandleInput() {

    }

    GoToNextState() {
        _player.State = _next_state
    }

    Update(){
        super.Update()
        if(super.IsStateFinished()){
            this.GoToNextState()
            //if alita is in range, attack again.
            //if not, go to chase alita
        }
    }

}




