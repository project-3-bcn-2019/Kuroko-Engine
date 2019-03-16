
import "ObjectLinker" for ObjectLinker,
EngineComunicator,
InputComunicator,
ComponentType

//For each var you declare, remember to create
//		setters [varname=(v) { __varname = v }]
//		and getters [varname { __varname }]
//The construct method is mandatory, do not erase!
//The import statement at the top og the cript is mandatory, do not erase!
//Be careful not to overwrite the methods declared in Game/ScriptingAPI/ObjectLinker.wren
//[gameObject] is a reserved identifier for the engine, don't use it for your own variables

class CinematicTrigger is ObjectLinker{

construct new(){}

    ui_tag {_ui_tag}
    ui_tag =(v){ _ui_tag = v}

    trigger_tag {_trigger_tag}
    trigger_tag =(v){ _trigger_tag = v}

 Start() {
    _component_collider = getComponent(ComponentType.PHYSICS)

    _game_objects = EngineComunicator.FindGameObjectsByTag(ui_tag)

    EngineComunicator.consoleOutput("Objs: %(_game_objects)")
    this.HideUI()

    _enabled = false
    _onCollision = false
 }

 Update() {

     //Controller
     if (_enabled) if (InputComunicator.getButton(-1,InputComunicator.C_X, InputComunicator.KEY_DOWN)) this.HideUI()

    //Keyboard
     if (_enabled) if (InputComunicator.getKey(InputComunicator.J, InputComunicator.KEY_DOWN)) this.HideUI()
     
    if (getCollisions().count > 0){

        for (obj in getCollisions()) { 
            
            if (obj.getTag() == trigger_tag){
                if (!_enabled) if (!_onCollision){
                         _onCollision = true
                        this.ShowUI()
                }       
                
            }  else {
                _onCollision = false
            } 
        }
    } else{
        _onCollision = false
    }
 }

 ShowUI(){

     for (obj in _game_objects) { 
        obj.setActive(true)
    }
    _enabled = true
 }

 HideUI(){

    for (obj in _game_objects) { 
         obj.setActive(false)
    }
    _enabled = false
 }


}