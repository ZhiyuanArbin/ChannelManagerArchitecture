# runCCCV Execution Flow

```mermaid
flowchart TD
    BatteryService["BatteryTestingService"]
    runCCCV["runCCCV()"]
    TaskQueue["Task Queue"]
    Worker1["Worker Thread 1"]
    Worker2["Worker Thread 2"]
    M4Thread["M4 Data Thread"]
    
    BatteryService --> runCCCV
    
    subgraph runCCCVSteps["runCCCV Steps"]
        Step1["Subscribe to channel data"]
        Step2["Create and add CC task to queue"]
        Step3["Register voltage monitoring callback"]
        Step4["Register step limit monitoring callback"]
        
        Step1 --> Step2 --> Step3 --> Step4
    end
    
    runCCCV --> runCCCVSteps
    runCCCVSteps --> TaskQueue
    
    TaskQueue --> Worker1
    TaskQueue --> Worker2
    
    subgraph WorkerThread1Tasks["Worker Thread 1 Tasks"]
        CCTask["CC Task"]
        CVTask["CV Task"]
        OtherCtrlTasks["Other control tasks"]
    end
    
    subgraph WorkerThread2Tasks["Worker Thread 2 Tasks"]
        CallbackTasks["Callback Tasks"]
        DataTasks["Data Tasks"]
        OtherTasks["Other tasks"]
    end
    
    subgraph M4ThreadTasks["M4 Data Thread Tasks"]
        UpdateData["Update channel data table"]
        CreateTasks["Create data processing tasks"]
        TriggerCBs["Trigger callbacks"]
    end
    
    Worker1 --> WorkerThread1Tasks
    Worker2 --> WorkerThread2Tasks
    M4Thread --> M4ThreadTasks