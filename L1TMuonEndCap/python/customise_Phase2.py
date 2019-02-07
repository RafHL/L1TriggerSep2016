import FWCore.ParameterSet.Config as cms

def customise(process):

    ## EMTF
    ## - see python/simEmtfDigis_cfi.py
    if hasattr(process, 'simEmtfDigis'):
        #process.simEmtfDigis.spPCParams16.ZoneBoundaries = [0,36,54,96,127]
        #process.simEmtfDigis.spPCParams16.UseNewZones    = True
        process.simEmtfDigis.DTEnable                    = True
        process.simEmtfDigis.CSCEnable                   = True
        process.simEmtfDigis.RPCEnable                   = True
        process.simEmtfDigis.GEMEnable                   = True
        process.simEmtfDigis.IRPCEnable                  = True
        process.simEmtfDigis.ME0Enable                   = True
        process.simEmtfDigis.Era                         = cms.string('Phase2_timing')
        process.simEmtfDigis.spPAParams16.PtLUTVersion   = cms.int32(7)

    ## EMTF conditions
    ## - see python/fakeEmtfParams_cff.py
    #if hasattr(process, 'emtfParams'):
    #    process.emtfParams.PtAssignVersion = cms.int32(7)
    #
    #if hasattr(process, 'emtfForestsDB'):
    #    process.emtfForestsDB = cms.ESSource(
    #        "EmptyESSource",
    #        recordName = cms.string('L1TMuonEndCapForestRcd'),
    #        iovIsRunNotTime = cms.bool(True),
    #        firstValid = cms.vuint32(1)
    #        )
    #
    #    process.emtfForests = cms.ESProducer(
    #        "L1TMuonEndCapForestESProducer",
    #        PtAssignVersion = cms.int32(7),
    #        bdtXMLDir = cms.string("2017_v7")
    #        )

    ## CSCTriggerPrimitives
    ## - revert to the old (i.e. Run 2) CSC LCT reconstruction
    ## - see L1Trigger/CSCTriggerPrimitives/python/cscTriggerPrimitiveDigis_cfi.py
    if hasattr(process, 'simCscTriggerPrimitiveDigis'):
        process.simCscTriggerPrimitiveDigis.commonParam = cms.PSet(
            # Master flag for SLHC studies
            isSLHC = cms.bool(True),

            # Debug
            verbosity = cms.int32(0),

            ## Whether or not to use the SLHC ALCT algorithm
            enableAlctSLHC = cms.bool(False),

            ## During Run-1, ME1a strips were triple-ganged
            ## Effectively, this means there were only 16 strips
            ## As of Run-2, ME1a strips are unganged,
            ## which increased the number of strips to 48
            gangedME1a = cms.bool(False),

            # flags to optionally disable finding stubs in ME42 or ME1a
            disableME1a = cms.bool(False),
            disableME42 = cms.bool(False),

            # offset between the ALCT and CLCT central BX in simulation
            alctClctOffset = cms.uint32(1),

            runME11Up = cms.bool(True),
            runME21Up = cms.bool(False),
            runME31Up = cms.bool(False),
            runME41Up = cms.bool(False),
        )

    ## RPCRecHit
    if hasattr(process, 'rpcRecHits'):
        process.rpcRecHits.rpcDigiLabel = 'simMuonRPCDigis'
    else:
        process.load('RecoLocalMuon.RPCRecHit.rpcRecHits_cfi')
        process.rpcRecHits.rpcDigiLabel = 'simMuonRPCDigis'

    return process
