import sys
import uuid
from PyQt5.QtCore import QDateTime
from PyQt5.QtNetwork import QTcpSocket
from PyQt5.QtWidgets import QApplication
from reapython.storage import fsStorage
from reapython.rea import scopeCache, stream, pipelines, pipeline, pipeParallel
from reapython.reaRemote import connectRemote, pipelineRemote
from reapython.server import normalServer
from time import sleep
import threading
import json

def readStorage(aRoot: str, aPath: str, aConfig: dict, aType: str) -> stream:
    return pipelines().input(False, "", scopeCache({"path": aPath, "config": aConfig}), True).asyncCall(aRoot + "read" + aType)

def writeStorage(aRoot: str, aPath: str, aConfig: dict, aType: str, aData) -> stream:
    return pipelines().input(False, "", scopeCache({"path": aPath, "config": aConfig, "data": aData}), True).asyncCall(aRoot + "write" + aType)

class dl:
    def __init__(self):
        self.__stgPath = "F:/3M/default/jobs.json"
        self.__stgConfig = {}
        self.__stgRoot = ""
        self.__whole_time = 1000
        self.__jobs = {}
        self.__running = set()
        self.__clients = dict()
        self.__job_mutex = threading.Lock()
        self.__socket_mutex = threading.Lock()

        self.serviceInternal()
        stm = readStorage(self.__stgRoot, self.__stgPath, self.__stgConfig, "JsonObject")
        if stm.data():
            self.__jobs = stm.scope().data("data")
            for i, j in self.__jobs.items():
                if j["status"] == "running":
                    if not self.startJob(i):
                        break
        
        def clientOnline(aInput: stream):
            with self.__socket_mutex:
                self.__clients[aInput.data()] = aInput.scope().data("socket")
                aInput.scope().cache("config", self.__stgConfig).cache("root", self.__stgRoot).cache("path", self.__stgPath)
            aInput.out()
        pipelines("server").add(clientOnline, {"name": "clientOnline", "external": "c++"})

        def clientOffline(aInput: stream):
            if not aInput.data():
                with self.__socket_mutex:
                    for i, j in self.__clients.items():
                        if aInput.scope().data("socket") == j:
                            del self.__clients[i]
                            break
        pipelines().find("clientStatusChanged").nextF(clientOffline)

        def startTrain(aInput: stream):
            usr = self.getUser(aInput.scope().data("socket"))
            dt = aInput.data()
            if usr == "":
                dt["err"] = 1
                dt["msg"] = "no this user!"
                aInput.setData(dt).out()
                return
            job = self.snowflakeID()
            pth = dt["path"]
            root = dt["root"]
            cfg = dt["config"]
            with self.__job_mutex:
                self.__jobs[job] = {"path": pth,
                                    "root": root,
                                    "config": cfg,
                                    "user": usr,
                                    "status": "running"}
                if writeStorage(self.__stgRoot, self.__stgPath, self.__stgConfig, "JsonObject", self.__jobs) and self.startJob(job):
                    dt["err"] = 0
                else:
                    del self.__jobs[job]
                    dt["err"] = 1
                    dt["msg"] = "start job failed!"
                aInput.setData(dt).out()
        pipelines("server").add(startTrain, {"name": "startTrain", "external": "qml"})

    def serviceInternal(self):        
        def execScript(aInput: stream):
            exec(aInput.data())
            aInput.out()
        pipelines("server").add(execScript, {"name": "evalPyScript",
            "external": "c++"})

        def doTraining(aInput: stream):
            tm = aInput.data()
            if tm > self.__whole_time:
                job = aInput.scope().data("job")
                with self.__job_mutex:
                    cfg = self.__jobs[job]
                    cfg["status"] = "success"
                    self.__jobs[job] = cfg
                    while not writeStorage(self.__stgRoot, self.__stgPath, self.__stgConfig, "JsonObject", self.__jobs).data():
                        sleep(0.005)
                    self.__running.remove(job)
            else:
                sleep(0.5)
                tm += 1
                aInput.outs(tm, "doTraining")
            usr = aInput.scope().data("user")
            with self.__socket_mutex:
                if usr in self.__clients:
                    pipelines("server").run("logTrain", 
                                            "time cost: " + str(tm) if tm < self.__whole_time else "complete", 
                                            "", 
                                            aInput.scope().cache("socket", self.__clients[usr]))
        pipelines().add(doTraining, {"name": "doTraining", "type": "Parallel"})
    
    def startJob(self, aJob: str) -> False:
        if len(self.__running) > 4:
            return False
        self.__running.add(aJob)
        cfg = self.__jobs[aJob].copy()
        cfg["job"] = aJob
        cfg["root0"] = self.__stgRoot
        cfg["path0"] = self.__stgPath
        cfg["config0"] = self.__stgConfig
        cfg["remote"] = True
        pipelines().run("doTraining", cfg.get("time", 0), "", scopeCache(cfg))
        return True
    
    def snowflakeID(self):
        return str(QDateTime().currentDateTime().toTime_t()) + "-" + str(uuid.uuid4())

    def getUser(self, aSocket: QTcpSocket)-> str:
        for i, j in self.__clients.items():
            if j == aSocket:
                return i
        return ""

def initServer():
    pipelines().add(lambda aInput:
        aInput.setData(pipeline("server")).out()
    , {"name": "createserverpipeline"})

    pipelines().add(lambda aInput:
        aInput.setData(pipelineRemote("c++", "server")).out()
    , {"name": "createc++pipeline"})

    pipelines().add(lambda aInput:
        aInput.setData(pipelineRemote("qml", "server")).out()
    , {"name": "createqmlpipeline"})

    server = normalServer()
    def writeRemote(aInput: stream):
        server.writeSocket(aInput.scope().data("socket"), aInput.data())
    connectRemote("server", "c++", writeRemote, False, "c++_server")
    connectRemote("server", "qml", writeRemote, False, "qml_server")

def main():
    app = QApplication(sys.argv)
    
    #init storage
    stg = fsStorage()
    stg.initialize()
    #init server sidecar
    initServer()
    #init control
    sv = dl()

    #test.stgTest()
    #test.doTest(1)

    # w = QWidget()
    # w.resize(250, 150)
    # w.move(300, 300)
    # w.setWindowTitle('Simple')
    # w.show()

    sys.exit(app.exec_())

if __name__ == '__main__':
    main()