classDiagram
    %% Main Service Classes
    class BatteryTestingService {
        -taskQueue: priority_queue<Task*>
        -workerThreads: vector<thread>
        -m4DataThread: thread
        -stopThreads: atomic<bool>
        -taskQueueMutex: mutex
        -taskQueueCV: condition_variable
        -channelCtrlService: ChannelCtrlService*
        -channelDataService: ChannelDataService*
        -callbackMap: map<uint32_t, vector<function>>
        +BatteryTestingService(numWorkerThreads)
        +~BatteryTestingService()
        +runCCCV(channel, current, targetVoltage, steplimit)
        +runCurrentRamp(channel, current)
        +runRest(channel)
        +setWorkerThreadCount(numThreads)
        +getWorkerThreadCount()
        -addTask(task)
        -registerCallback(channel, callback)
        -handleCallbacks(channel)
        -unregisterCallback(channel, callbackIndex)
        -workerThreadFunction()
        -m4DataThreadFunction()
    }

    %% Task Hierarchy
    class Task {
        +priority: TaskPriority
        +Task(priority)
        +~Task()
        +execute()*
    }
    
    class ControlTask {
        +ControlTask(priority)
        +~ControlTask()
    }
    
    class DataTask {
        +DataTask(priority)
        +~DataTask()
    }
    
    class CCTask {
        -channel: uint32_t
        -current: float
        -ctrlService: ChannelCtrlService*
        +CCTask(channel, current, ctrlService)
        +execute()
    }
    
    class CVTask {
        -channel: uint32_t
        -targetVoltage: float
        -ctrlService: ChannelCtrlService*
        +CVTask(channel, targetVoltage, ctrlService)
        +execute()
    }
    
    class CallbackControlTask {
        -channel: uint32_t
        -callback: CallbackFunction
        -dataService: ChannelDataService*
        +CallbackControlTask(channel, callback, dataService)
        +execute()
    }
    
    class GenericControlTask {
        -ctrlServices: queue<ChannelCtrlService*>
        +GenericControlTask(priority)
        +~GenericControlTask()
        +execute()
        +addCtrlService(service)
    }
    
    class FilteringDataTask {
        -channel: uint32_t
        -rawData: map<string, float>
        +FilteringDataTask(channel, data)
        +execute()
    }
    
    class FittingDataTask {
        -channel: uint32_t
        -rawData: map<string, float>
        +FittingDataTask(channel, data)
        +execute()
    }
    
    %% Channel Service Hierarchy
    class ChannelCtrlService {
        +ChannelCtrlService()
        +~ChannelCtrlService()
        +doConstantCurrent(channel, current)*
        +doConstantVoltage(channel, voltage)*
        +doRest(channel)*
        +doOFF(channel)*
    }
    
    class ChannelDataService {
        +ChannelDataService()
        +~ChannelDataService()
        +subscribeChannel(channel)*
        +unsubscribeChannel(channel)*
        +isChannelSubscribed(channel)*
        +getVoltage(channel)*
        +getCurrent(channel)*
        +getDvDt(channel)*
        +getChannelData(channel)*
        +receiveM4Data(channel, data)*
    }
    
    class DummyChannelCtrlService {
        +doConstantCurrent(channel, current)
        +doConstantVoltage(channel, voltage)
        +doRest(channel)
        +doOFF(channel)
    }
    
    class DummyChannelDataService {
        -channelDataTable: map<uint32_t, map<string, float>>
        -subscribedChannels: map<uint32_t, bool>
        +subscribeChannel(channel)
        +unsubscribeChannel(channel)
        +isChannelSubscribed(channel)
        +getVoltage(channel)
        +getCurrent(channel)
        +getDvDt(channel)
        +getChannelData(channel)
        +receiveM4Data(channel, data)
    }
    
    %% Relationships
    Task <|-- ControlTask : inherits
    Task <|-- DataTask : inherits
    
    ControlTask <|-- CCTask : inherits
    ControlTask <|-- CVTask : inherits
    ControlTask <|-- CallbackControlTask : inherits
    ControlTask <|-- GenericControlTask : inherits
    
    DataTask <|-- FilteringDataTask : inherits
    DataTask <|-- FittingDataTask : inherits
    
    ChannelCtrlService <|-- DummyChannelCtrlService : inherits
    ChannelDataService <|-- DummyChannelDataService : inherits
    
    BatteryTestingService --> Task : manages
    BatteryTestingService --> ChannelCtrlService : uses
    BatteryTestingService --> ChannelDataService : uses