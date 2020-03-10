
abstract case class GhdlWaveFormat(idx : Int)
object GhdlNone  extends GhdlWaveFormat(0)
object GhdlVcd   extends GhdlWaveFormat(1)
object GhdlVcdgz extends GhdlWaveFormat(2)
object GhdlFst   extends GhdlWaveFormat(3)
object GhdlGhw   extends GhdlWaveFormat(4)


class SimRawWrapper {

  @native def init(simulationPath : String, 
                   waveFormat : GhdlWaveFormat, 
                   wavePath : String)

  @native def getHandle(handleName : String) : Long
  @native def getInt(handle : Long) : Int
  @native def getLong(handle : Long) : Long
  @native def getBigInt(handle : Long) : Array[Byte]
  @native def setInt(handle : Long, value : Int) : Unit
  @native def setLong(handle : Long, value : Long) : Unit
  @native def setBigInt(handle : Long, value : Array[Byte]) : Unit
  @native def eval() : Unit
  @native def sleep(cycles : Long) : Unit
  @native def end() : Unit
  @native override def finalize() : Unit

}
