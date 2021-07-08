#ifndef _TM_REGISTERS_DESCRIPTION_H_
#define _TM_REGISTERS_DESCRIPTION_H_

#define FOO(...) __VA_ARGS__
#define foo(x) x

/** Synced to Cider \EBU-IP \TXQ_IP \SIP7.0 (Falcon) \TXQ_PR_IP \TXQ_PR_IP {7.0.10} \TXQ_PSI \TXQ_PDQ 
 */


#define		TM_Sched_EccErrStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( CorrEccErr, 16, 6 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncEccErr, 0, 6 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ScrubDis_Addr(operation, ...) \
    TM_REG_ROW_##operation( Dis, 0, 1 , 0x0, __VA_ARGS__)\

#define		TM_Sched_Plast_Addr(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x1, __VA_ARGS__)\

#define		TM_Sched_PPerRateShapInt_Addr(operation, ...) \
    TM_REG_ROW_##operation( I, 32, 14 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( B, 0, 14 , 0x1, __VA_ARGS__)\

#define		TM_Sched_PMemsEccErrStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( CorrEccErr, 16, 2 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncEccErr, 0, 2 , 0x0, __VA_ARGS__)\

#define		TM_Sched_PBnkEccErrStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( CorrEccErr, 16, 2 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncEccErr, 0, 2 , 0x0, __VA_ARGS__)\

#define		TM_Sched_PPerStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( PerPointer, 0, 6 , 0x0, __VA_ARGS__)\

#define		TM_Sched_CPerRateShapInt_Addr(operation, ...) \
    TM_REG_ROW_##operation( I, 32, 14 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( B, 0, 14 , 0x1, __VA_ARGS__)\

#define		TM_Sched_CMemsEccErrStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( CorrEccErr, 16, 5 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncEccErr, 0, 5 , 0x0, __VA_ARGS__)\

#define		TM_Sched_CMyQEccErrStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( CorrEccErr, 16, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncEccErr, 0, 1 , 0x0, __VA_ARGS__)\

#define		TM_Sched_CBnkEccErrStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( CorrEccErr, 16, 4 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncEccErr, 0, 4 , 0x0, __VA_ARGS__)\

#define		TM_Sched_CPerStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( PerPointer, 32, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerRoundCnt, 0, 10 , 0x0, __VA_ARGS__)\

#define		TM_Sched_BPerRateShapInt_Addr(operation, ...) \
    TM_REG_ROW_##operation( I, 32, 14 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( B, 0, 14 , 0x1, __VA_ARGS__)\

#define		TM_Sched_BMemsEccErrStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( CorrEccErr, 16, 6 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncEccErr, 0, 6 , 0x0, __VA_ARGS__)\

#define		TM_Sched_BMyQEccErrStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( CorrEccErr, 16, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncEccErr, 0, 1 , 0x0, __VA_ARGS__)\

#define		TM_Sched_BBnkEccErrStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( CorrEccErr, 16, 5 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncEccErr, 0, 5 , 0x0, __VA_ARGS__)\

#define		TM_Sched_BPerStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( PerPointer, 32, 11 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerRoundCnt, 0, 10 , 0x0, __VA_ARGS__)\

#define		TM_Sched_APerRateShapInt_Addr(operation, ...) \
    TM_REG_ROW_##operation( I, 32, 14 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( B, 0, 14 , 0x1, __VA_ARGS__)\

#define		TM_Sched_AMemsEccErrStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( CorrEccErr, 16, 7 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncEccErr, 0, 7 , 0x0, __VA_ARGS__)\

#define		TM_Sched_AMyQEccErrStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( CorrEccErr, 16, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncEccErr, 0, 1 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ABnkEccErrStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( CorrEccErr, 16, 5 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncEccErr, 0, 5 , 0x0, __VA_ARGS__)\

#define		TM_Sched_APerStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( PerPointer, 32, 11 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerRoundCnt, 0, 10 , 0x0, __VA_ARGS__)\

#define		TM_Sched_QMemsEccErrStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( CorrEccErr, 16, 7 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncEccErr, 0, 7 , 0x0, __VA_ARGS__)\

#define		TM_Sched_QPerRateShapInt_Addr(operation, ...) \
    TM_REG_ROW_##operation( I, 32, 14 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( B, 0, 14 , 0x1, __VA_ARGS__)\

#define		TM_Sched_QBnkEccErrStatus_StartAddr(operation, ...) \
    TM_REG_ROW_##operation( CorrEccErr, 16, 5 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncEccErr, 0, 5 , 0x0, __VA_ARGS__)\

#define		TM_Sched_QueueBank1EccErrStatus(operation, ...) \
    TM_REG_ROW_##operation( CorrEccErr, 16, 5 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncEccErr, 0, 5 , 0x0, __VA_ARGS__)\

#define		TM_Sched_QueueBank2EccErrStatus(operation, ...) \
    TM_REG_ROW_##operation( CorrEccErr, 16, 5 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncEccErr, 0, 5 , 0x0, __VA_ARGS__)\

#define		TM_Sched_QueueBank3EccErrStatus(operation, ...) \
    TM_REG_ROW_##operation( CorrEccErr, 16, 5 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncEccErr, 0, 5 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ErrorStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( FIFOOvrflowErr, 5, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( TBNegSat, 4, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( BPBSat, 3, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncECCErr, 2, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( CorrECCErr, 1, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( ForcedErr, 0, 1 , 0x0, __VA_ARGS__)\

#define		TM_Sched_FirstExcp_Addr(operation, ...) \
    TM_REG_ROW_##operation( FIFOOvrflowErr, 5, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( TBNegSat, 4, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( BPBSat, 3, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncECCErr, 2, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( CorrECCErr, 1, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( ForcedErr, 0, 1 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ErrCnt_Addr(operation, ...) \
    TM_REG_ROW_##operation( Cnt, 0, 16 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ExcpCnt_Addr(operation, ...) \
    TM_REG_ROW_##operation( Cnt, 0, 16 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ExcpMask_Addr(operation, ...) \
    TM_REG_ROW_##operation( ECOReg, 32, 32 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( FIFOOvrflowErr, 5, 1 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( TBNegSat, 4, 1 , 0x3, __VA_ARGS__)\
    TM_REG_ROW_##operation( BPBSat, 3, 1 , 0x2, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncECCErr, 2, 1 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( CorrECCErr, 1, 1 , 0x3, __VA_ARGS__)\
    TM_REG_ROW_##operation( ForcedErr, 0, 1 , 0x3, __VA_ARGS__)\

#define		TM_Sched_Identity_Addr(operation, ...) \
    TM_REG_ROW_##operation( SUID, 8, 8 , 0x10, __VA_ARGS__)\
    TM_REG_ROW_##operation( UID, 0, 8 , 0x33, __VA_ARGS__)\

#define		TM_Sched_ForceErr_Addr(operation, ...) \
    TM_REG_ROW_##operation( ForcedErr, 0, 1 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ScrubSlots_Addr(operation, ...) \
    TM_REG_ROW_##operation( PortSlots, 32, 6 , 0x4, __VA_ARGS__)\
    TM_REG_ROW_##operation( ClvlSlots, 24, 6 , 0x4, __VA_ARGS__)\
    TM_REG_ROW_##operation( BlvlSlots, 16, 6 , 0x8, __VA_ARGS__)\
    TM_REG_ROW_##operation( AlvlSlots, 8, 6 , 0x10, __VA_ARGS__)\
    TM_REG_ROW_##operation( QueueSlots, 0, 6 , 0x20, __VA_ARGS__)\

#define		TM_Sched_BPMsgFIFO_Addr(operation, ...) \
    TM_REG_ROW_##operation( SetThresh, 32, 5 , 0x8, __VA_ARGS__)\
    TM_REG_ROW_##operation( ClrThresh, 0, 5 , 0x7, __VA_ARGS__)\

#define		TM_Sched_TreeDeqEn_Addr(operation, ...) \
    TM_REG_ROW_##operation( En, 0, 1 , 0x1, __VA_ARGS__)\

#define		TM_Sched_PDWRREnReg(operation, ...) \
    TM_REG_ROW_##operation( PrioEn, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_PPerCtlConf_Addr(operation, ...) \
    TM_REG_ROW_##operation( DecEn, 48, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerInterval, 16, 8 , 0x50, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerEn, 0, 1 , 0x0, __VA_ARGS__)\

#define		TM_Sched_PPerRateShap_Addr(operation, ...) \
    TM_REG_ROW_##operation( L, 32, 14 , 0x16e, __VA_ARGS__)\
    TM_REG_ROW_##operation( K, 16, 14 , 0x66f, __VA_ARGS__)\
    TM_REG_ROW_##operation( N, 0, 14 , 0x2, __VA_ARGS__)\

#define		TM_Sched_PortExtBPEn_Addr(operation, ...) \
    TM_REG_ROW_##operation( En, 0, 1 , 0x1, __VA_ARGS__)\

#define		TM_Sched_PBytePerBurstLimit_Addr(operation, ...) \
    TM_REG_ROW_##operation( limit, 0, 16 , 0x1001, __VA_ARGS__)\

#define		TM_Sched_CPerCtlConf_Addr(operation, ...) \
    TM_REG_ROW_##operation( DecEn, 48, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerInterval, 16, 12 , 0x71c, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerEn, 0, 1 , 0x0, __VA_ARGS__)\

#define		TM_Sched_CPerRateShap_Addr(operation, ...) \
    TM_REG_ROW_##operation( L, 32, 14 , 0x16e, __VA_ARGS__)\
    TM_REG_ROW_##operation( K, 16, 14 , 0x66f, __VA_ARGS__)\
    TM_REG_ROW_##operation( N, 0, 14 , 0x2, __VA_ARGS__)\

#define		TM_Sched_BPerCtlConf_Addr(operation, ...) \
    TM_REG_ROW_##operation( DecEn, 48, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerInterval, 16, 12 , 0xe39, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerEn, 0, 1 , 0x0, __VA_ARGS__)\

#define		TM_Sched_BPerRateShap_Addr(operation, ...) \
    TM_REG_ROW_##operation( L, 32, 14 , 0x16e, __VA_ARGS__)\
    TM_REG_ROW_##operation( K, 16, 14 , 0x66f, __VA_ARGS__)\
    TM_REG_ROW_##operation( N, 0, 14 , 0x2, __VA_ARGS__)\

#define		TM_Sched_APerCtlConf_Addr(operation, ...) \
    TM_REG_ROW_##operation( DecEn, 48, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerInterval, 16, 12 , 0xe39, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerEn, 0, 1 , 0x0, __VA_ARGS__)\

#define		TM_Sched_APerRateShap_Addr(operation, ...) \
    TM_REG_ROW_##operation( L, 32, 14 , 0x16e, __VA_ARGS__)\
    TM_REG_ROW_##operation( K, 16, 14 , 0x66f, __VA_ARGS__)\
    TM_REG_ROW_##operation( N, 0, 14 , 0x2, __VA_ARGS__)\

#define		TM_Sched_QPerCtlConf_Addr(operation, ...) \
    TM_REG_ROW_##operation( DecEn, 48, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerInterval, 16, 12 , 0xe39, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerEn, 0, 1 , 0x0, __VA_ARGS__)\

#define		TM_Sched_QPerRateShap_Addr(operation, ...) \
    TM_REG_ROW_##operation( L, 32, 14 , 0x16e, __VA_ARGS__)\
    TM_REG_ROW_##operation( K, 16, 14 , 0x66f, __VA_ARGS__)\
    TM_REG_ROW_##operation( N, 0, 14 , 0x2, __VA_ARGS__)\

#define		TM_Sched_EccConfig_Addr(operation, ...) \
    TM_REG_ROW_##operation( QGrandParentErrInsEnable, 45, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( AGrandParentErrInsEnable, 44, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( BGrandParentErrInsEnable, 43, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( QStateErrInsEnable, 42, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( AStateErrInsEnable, 41, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( QFuncErrInsEnable, 40, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( QLastErrInsEnable, 39, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( ALastErrInsEnable, 38, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( BLastErrInsEnable, 37, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( CLastErrInsEnable, 36, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( AParentErrInsEnable, 35, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( BParentErrInsEnable, 34, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( CParentErrInsEnable, 33, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( ADWRRErrInsEnable, 32, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( BDWRRErrInsEnable, 31, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( CDWRRErrInsEnable, 30, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PDWRRErrInsEnable, 29, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( AMyQErrInsEnable, 28, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( BMyQErrInsEnable, 27, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( CMyQErrInsEnable, 26, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( QWFSErrInsEnable, 21, 4 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( QTBNeg2ErrInsEnable, 17, 4 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( QTBNegErrInsEnable, 13, 4 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( BTBErrInsEnable, 7, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( CTBErrInsEnable, 6, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( BPerConfErrInsEnable, 5, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( CPerConfErrInsEnable, 4, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( AIDErrInsEnable, 3, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( QtoAErrInsEnable, 2, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( ErrInsMode, 1, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( LockFirstErronousEvent, 0, 1 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters0(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters1(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters2(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters3(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters4(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters5(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters6(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters7(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters8(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters9(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters10(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters11(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters12(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters13(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters14(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters15(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters16(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters17(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters18(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters19(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters20(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters21(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters22(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters23(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters24(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters25(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters26(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters27(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters28(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters29(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters30(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters31(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters32(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters33(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters34(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters35(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters36(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ECC_Memory_Parameters37(operation, ...) \
    TM_REG_ROW_##operation( Syndrom, 32, 32 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Address, 8, 24 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Counter, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_PortNodeState(operation, ...) \
    TM_REG_ROW_##operation( State, 0, 18 , 0x1fe, __VA_ARGS__)\

#define		TM_Sched_PortMyQ(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 35 , 0x0, __VA_ARGS__)\

#define		TM_Sched_PortRRDWRRStatus01(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 14 , 0x3fff, __VA_ARGS__)\

#define		TM_Sched_PortRRDWRRStatus23(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 14 , 0x3fff, __VA_ARGS__)\

#define		TM_Sched_PortWFS(operation, ...) \
    TM_REG_ROW_##operation( WFS, 0, 32 , 0x0, __VA_ARGS__)\

#define		TM_Sched_PortBPFromSTF(operation, ...) \
    TM_REG_ROW_##operation( BP, 0, 64 , 0x0, __VA_ARGS__)\

#define		TM_Sched_PortBPFromQMgr(operation, ...) \
    TM_REG_ROW_##operation( BP, 0, 64 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ClvlNodeState(operation, ...) \
    TM_REG_ROW_##operation( State, 0, 11 , 0x2, __VA_ARGS__)\

#define		TM_Sched_ClvlMyQ(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 29 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ClvlRRDWRRStatus01(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 14 , 0x3fff, __VA_ARGS__)\

#define		TM_Sched_ClvlRRDWRRStatus23(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 14 , 0x3fff, __VA_ARGS__)\

#define		TM_Sched_ClvlWFS(operation, ...) \
    TM_REG_ROW_##operation( WFS, 0, 32 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ClvlL0ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ClvlBPFromSTF(operation, ...) \
    TM_REG_ROW_##operation( BP, 0, 64 , 0x0, __VA_ARGS__)\

#define		TM_Sched_TMtoTMClvlBPState(operation, ...) \
    TM_REG_ROW_##operation( BPState, 0, 1 , 0x0, __VA_ARGS__)\

#define		TM_Sched_BlvlNodeState(operation, ...) \
    TM_REG_ROW_##operation( State, 0, 11 , 0x2, __VA_ARGS__)\

#define		TM_Sched_BlvlMyQ(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 23 , 0x0, __VA_ARGS__)\

#define		TM_Sched_BlvlRRDWRRStatus01(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 18 , 0x3ffff, __VA_ARGS__)\

#define		TM_Sched_BlvlRRDWRRStatus23(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 18 , 0x3ffff, __VA_ARGS__)\

#define		TM_Sched_BlvlWFS(operation, ...) \
    TM_REG_ROW_##operation( WFS, 0, 32 , 0x0, __VA_ARGS__)\

#define		TM_Sched_BlvlL0ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		TM_Sched_BlvlL1ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		TM_Sched_BlvlL1ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		TM_Sched_AlvlNodeState(operation, ...) \
    TM_REG_ROW_##operation( State, 0, 11 , 0x2, __VA_ARGS__)\

#define		TM_Sched_AlvlMyQ(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 17 , 0x0, __VA_ARGS__)\

#define		TM_Sched_AlvlRRDWRRStatus01(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 22 , 0x3fffff, __VA_ARGS__)\

#define		TM_Sched_AlvlWFS(operation, ...) \
    TM_REG_ROW_##operation( WFS, 0, 32 , 0x0, __VA_ARGS__)\

#define		TM_Sched_AlvlL0ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		TM_Sched_AlvlL1ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		TM_Sched_AlvlL1ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		TM_Sched_AlvlL2ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		TM_Sched_AlvlL2ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		TM_Sched_QueueNodeState(operation, ...) \
    TM_REG_ROW_##operation( State, 0, 11 , 0x2, __VA_ARGS__)\

#define		TM_Sched_QueueWFS(operation, ...) \
    TM_REG_ROW_##operation( WFS, 0, 32 , 0x0, __VA_ARGS__)\

#define		TM_Sched_QueueL0ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		TM_Sched_QueueL1ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		TM_Sched_QueueL1ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		TM_Sched_QueueL2ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		TM_Sched_QueueL2ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		TM_Sched_TMtoTMQueueBPState(operation, ...) \
    TM_REG_ROW_##operation( BPState, 0, 1 , 0x0, __VA_ARGS__)\

#define		TM_Sched_QueuePerStatus(operation, ...) \
    TM_REG_ROW_##operation( PerPointer, 32, 11 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerRoundCnt, 0, 10 , 0x0, __VA_ARGS__)\

#define		TM_Sched_PortEligPrioFunc_Entry(operation, ...) \
    TM_REG_ROW_##operation( FuncOut3, 48, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut2, 32, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut1, 16, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut0, 0, 9 , 0x0, __VA_ARGS__)\

#define		TM_Sched_PortEligPrioFuncPtr(operation, ...) \
    TM_REG_ROW_##operation( Ptr, 0, 6 , 0x0, __VA_ARGS__)\

#define		TM_Sched_PortTokenBucketTokenEnDiv(operation, ...) \
    TM_REG_ROW_##operation( Periods, 32, 13 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxTokenRes, 28, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxToken, 16, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTokenRes, 12, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinToken, 0, 12 , 0xfff, __VA_ARGS__)\

#define		TM_Sched_PortTokenBucketBurstSize(operation, ...) \
    TM_REG_ROW_##operation( MaxBurstSz, 32, 17 , 0x1ffff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinBurstSz, 0, 17 , 0x1ffff, __VA_ARGS__)\

#define		TM_Sched_PortDWRRPrioEn(operation, ...) \
    TM_REG_ROW_##operation( En, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_PortQuantumsPriosLo(operation, ...) \
    TM_REG_ROW_##operation( Quantum3, 48, 12 , 0x10, __VA_ARGS__)\
    TM_REG_ROW_##operation( Quantum2, 32, 12 , 0x10, __VA_ARGS__)\
    TM_REG_ROW_##operation( Quantum1, 16, 12 , 0x10, __VA_ARGS__)\
    TM_REG_ROW_##operation( Quantum0, 0, 12 , 0x10, __VA_ARGS__)\

#define		TM_Sched_PortQuantumsPriosHi(operation, ...) \
    TM_REG_ROW_##operation( Quantum7, 48, 12 , 0x10, __VA_ARGS__)\
    TM_REG_ROW_##operation( Quantum6, 32, 12 , 0x10, __VA_ARGS__)\
    TM_REG_ROW_##operation( Quantum5, 16, 12 , 0x10, __VA_ARGS__)\
    TM_REG_ROW_##operation( Quantum4, 0, 12 , 0x10, __VA_ARGS__)\

#define		TM_Sched_PortRangeMap(operation, ...) \
    TM_REG_ROW_##operation( Hi, 24, 7 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Lo, 0, 7 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ClvlEligPrioFunc_Entry(operation, ...) \
    TM_REG_ROW_##operation( FuncOut3, 48, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut2, 32, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut1, 16, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut0, 0, 9 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ClvlEligPrioFuncPtr(operation, ...) \
    TM_REG_ROW_##operation( Ptr, 0, 6 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ClvlTokenBucketTokenEnDiv(operation, ...) \
    TM_REG_ROW_##operation( MaxDivExp, 40, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinDivExp, 32, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxTokenRes, 28, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxToken, 16, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTokenRes, 12, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinToken, 0, 12 , 0xfff, __VA_ARGS__)\

#define		TM_Sched_ClvlTokenBucketBurstSize(operation, ...) \
    TM_REG_ROW_##operation( MaxBurstSz, 32, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinBurstSz, 0, 12 , 0xfff, __VA_ARGS__)\

#define		TM_Sched_ClvlDWRRPrioEn(operation, ...) \
    TM_REG_ROW_##operation( En, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_ClvlQuantum(operation, ...) \
    TM_REG_ROW_##operation( Quantum, 0, 14 , 0x40, __VA_ARGS__)\

#define		TM_Sched_ClvltoPortAndBlvlRangeMap(operation, ...) \
    TM_REG_ROW_##operation( Port, 48, 7 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( BlvlHi, 24, 7 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( BlvlLo, 0, 7 , 0x0, __VA_ARGS__)\

#define		TM_Sched_BlvlEligPrioFunc_Entry(operation, ...) \
    TM_REG_ROW_##operation( FuncOut3, 48, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut2, 32, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut1, 16, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut0, 0, 9 , 0x0, __VA_ARGS__)\

#define		TM_Sched_BlvlEligPrioFuncPtr(operation, ...) \
    TM_REG_ROW_##operation( Ptr, 0, 6 , 0x0, __VA_ARGS__)\

#define		TM_Sched_BlvlTokenBucketTokenEnDiv(operation, ...) \
    TM_REG_ROW_##operation( MaxDivExp, 40, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinDivExp, 32, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxTokenRes, 28, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxToken, 16, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTokenRes, 12, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinToken, 0, 12 , 0xfff, __VA_ARGS__)\

#define		TM_Sched_BlvlTokenBucketBurstSize(operation, ...) \
    TM_REG_ROW_##operation( MaxBurstSz, 32, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinBurstSz, 0, 12 , 0xfff, __VA_ARGS__)\

#define		TM_Sched_BlvlDWRRPrioEn(operation, ...) \
    TM_REG_ROW_##operation( En, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_BlvlQuantum(operation, ...) \
    TM_REG_ROW_##operation( Quantum, 0, 14 , 0x40, __VA_ARGS__)\

#define		TM_Sched_BLvltoClvlAndAlvlRangeMap(operation, ...) \
    TM_REG_ROW_##operation( Clvl, 48, 7 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( AlvlHi, 24, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( AlvlLo, 0, 9 , 0x0, __VA_ARGS__)\

#define		TM_Sched_AlvlEligPrioFunc_Entry(operation, ...) \
    TM_REG_ROW_##operation( FuncOut3, 48, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut2, 32, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut1, 16, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut0, 0, 9 , 0x0, __VA_ARGS__)\

#define		TM_Sched_AlvlEligPrioFuncPtr(operation, ...) \
    TM_REG_ROW_##operation( Ptr, 0, 6 , 0x0, __VA_ARGS__)\

#define		TM_Sched_AlvlTokenBucketTokenEnDiv(operation, ...) \
    TM_REG_ROW_##operation( MaxDivExp, 40, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinDivExp, 32, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxTokenRes, 28, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxToken, 16, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTokenRes, 12, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinToken, 0, 12 , 0xfff, __VA_ARGS__)\

#define		TM_Sched_AlvlTokenBucketBurstSize(operation, ...) \
    TM_REG_ROW_##operation( MaxBurstSz, 32, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinBurstSz, 0, 12 , 0xfff, __VA_ARGS__)\

#define		TM_Sched_AlvlDWRRPrioEn(operation, ...) \
    TM_REG_ROW_##operation( En, 0, 8 , 0x0, __VA_ARGS__)\

#define		TM_Sched_AlvlQuantum(operation, ...) \
    TM_REG_ROW_##operation( Quantum, 0, 14 , 0x40, __VA_ARGS__)\

#define		TM_Sched_ALvltoBlvlAndQueueRangeMap(operation, ...) \
    TM_REG_ROW_##operation( Blvl, 48, 7 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( QueueHi, 24, 11 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( QueueLo, 0, 11 , 0x0, __VA_ARGS__)\

#define		TM_Sched_QueueEligPrioFunc(operation, ...) \
    TM_REG_ROW_##operation( FuncOut3, 48, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut2, 32, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut1, 16, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut0, 0, 9 , 0x0, __VA_ARGS__)\

#define		TM_Sched_QueueEligPrioFuncPtr(operation, ...) \
    TM_REG_ROW_##operation( Ptr, 0, 6 , 0x0, __VA_ARGS__)\

#define		TM_Sched_QueueTokenBucketTokenEnDiv(operation, ...) \
    TM_REG_ROW_##operation( MaxDivExp, 40, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinDivExp, 32, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxTokenRes, 28, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxToken, 16, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTokenRes, 12, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinToken, 0, 12 , 0xfff, __VA_ARGS__)\

#define		TM_Sched_QueueTokenBucketBurstSize(operation, ...) \
    TM_REG_ROW_##operation( MaxBurstSz, 32, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinBurstSz, 0, 12 , 0xfff, __VA_ARGS__)\

#define		TM_Sched_QueueQuantum(operation, ...) \
    TM_REG_ROW_##operation( Quantum, 0, 14 , 0x40, __VA_ARGS__)\

#define		TM_Sched_QueueAMap(operation, ...) \
    TM_REG_ROW_##operation( Alvl, 0, 9 , 0x0, __VA_ARGS__)\

#define		TM_Sched_PortShpBucketLvls(operation, ...) \
    TM_REG_ROW_##operation( MaxLvl, 32, 28 , 0x7ffffff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinLvl, 0, 28 , 0x7ffffff, __VA_ARGS__)\

#define		TM_Sched_ClvlShpBucketLvls(operation, ...) \
    TM_REG_ROW_##operation( MaxLvl, 32, 23 , 0x3fffff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinLvl, 0, 23 , 0x3fffff, __VA_ARGS__)\

#define		TM_Sched_CLvlDef(operation, ...) \
    TM_REG_ROW_##operation( Deficit, 0, 22 , 0x1, __VA_ARGS__)\

#define		TM_Sched_BlvlShpBucketLvls(operation, ...) \
    TM_REG_ROW_##operation( MaxLvl, 32, 23 , 0x3fffff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinLvl, 0, 23 , 0x3fffff, __VA_ARGS__)\

#define		TM_Sched_BlvlDef(operation, ...) \
    TM_REG_ROW_##operation( Deficit, 0, 22 , 0x1, __VA_ARGS__)\

#define		TM_Sched_AlvlShpBucketLvls(operation, ...) \
    TM_REG_ROW_##operation( MaxLvl, 32, 23 , 0x3fffff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinLvl, 0, 23 , 0x3fffff, __VA_ARGS__)\

#define		TM_Sched_AlvlDef(operation, ...) \
    TM_REG_ROW_##operation( Deficit, 0, 22 , 0x1, __VA_ARGS__)\

#define		TM_Sched_QueueShpBucketLvls(operation, ...) \
    TM_REG_ROW_##operation( MaxLvl, 32, 23 , 0x3fffff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinLvl, 0, 23 , 0x3fffff, __VA_ARGS__)\

#define		TM_Sched_QueueDef(operation, ...) \
    TM_REG_ROW_##operation( Deficit, 0, 22 , 0x1, __VA_ARGS__)\

#define		TM_Sched_PortShaperBucketNeg(operation, ...) \
    TM_REG_ROW_##operation( MaxNeg, 32, 16 , 0xffff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTBNeg, 0, 16 , 0xffff, __VA_ARGS__)\

#define		TM_Sched_CLevelShaperBucketNeg(operation, ...) \
    TM_REG_ROW_##operation( MaxTBNeg, 32, 16 , 0xffff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTBNeg, 0, 16 , 0xffff, __VA_ARGS__)\

#define		TM_Sched_BLevelShaperBucketNeg(operation, ...) \
    TM_REG_ROW_##operation( MaxTBNeg, 32, 32 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTBNeg, 0, 32 , 0x1, __VA_ARGS__)\

#define		TM_Sched_ALevelShaperBucketNeg(operation, ...) \
    TM_REG_ROW_##operation( MaxTBNeg, 32, 32 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTBNeg, 0, 32 , 0x1, __VA_ARGS__)\

#define		TM_Sched_QueueShaperBucketNeg(operation, ...) \
    TM_REG_ROW_##operation( MaxTBNeg, 32, 32 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTBNeg, 0, 32 , 0x1, __VA_ARGS__)\

#define		TM_Sched_PortRRDWRRStatus67(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 14 , 0x3fff, __VA_ARGS__)\

#define		TM_Sched_PortRRDWRRStatus45(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 14 , 0x3fff, __VA_ARGS__)\

#define		TM_Sched_ClvlRRDWRRStatus67(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 14 , 0x3fff, __VA_ARGS__)\

#define		TM_Sched_ClvlRRDWRRStatus45(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 14 , 0x3fff, __VA_ARGS__)\

#define		TM_Sched_ClvlL0ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( status, 0, 64 , 0x0, __VA_ARGS__)\

#define		TM_Sched_BlvlRRDWRRStatus67(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 18 , 0x3ffff, __VA_ARGS__)\

#define		TM_Sched_BlvlRRDWRRStatus45(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 18 , 0x3ffff, __VA_ARGS__)\

#define		TM_Sched_BlvlL0ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( status, 0, 64 , 0x0, __VA_ARGS__)\

#define		TM_Sched_AlvlRRDWRRStatus67(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 22 , 0x3fffff, __VA_ARGS__)\

#define		TM_Sched_AlvlRRDWRRStatus45(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 22 , 0x3fffff, __VA_ARGS__)\

#define		TM_Sched_AlvlL0ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( status, 0, 64 , 0x0, __VA_ARGS__)\

#define		TM_Sched_QueueL0ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( status, 0, 64 , 0x0, __VA_ARGS__)\

#define		TM_Sched_TMtoTMPortBPState(operation, ...) \
    TM_REG_ROW_##operation( BP, 0, 64 , 0x0, __VA_ARGS__)\

#define		TM_Sched_TMtoTMBlvlBPState(operation, ...) \
    TM_REG_ROW_##operation( BPState, 0, 1 , 0x0, __VA_ARGS__)\

#define		TM_Sched_AlvlRRDWRRStatus23(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 22 , 0x3fffff, __VA_ARGS__)\

#define		TM_Sched_TMtoTMAlvlBPState(operation, ...) \
    TM_REG_ROW_##operation( BPState, 0, 1 , 0x0, __VA_ARGS__)\

#define		TM_Sched_PortDefPrio0(operation, ...) \
    TM_REG_ROW_##operation( Deficit, 0, 19 , 0x1, __VA_ARGS__)\

#define		TM_Sched_PortDefPrio1(operation, ...) \
    TM_REG_ROW_##operation( Deficit, 0, 19 , 0x1, __VA_ARGS__)\

#define		TM_Sched_PortDefPrio2(operation, ...) \
    TM_REG_ROW_##operation( Deficit, 0, 19 , 0x1, __VA_ARGS__)\

#define		TM_Sched_PortDefPrio3(operation, ...) \
    TM_REG_ROW_##operation( Deficit, 0, 19 , 0x1, __VA_ARGS__)\

#define		TM_Sched_PortDefPrio4(operation, ...) \
    TM_REG_ROW_##operation( Deficit, 0, 19 , 0x1, __VA_ARGS__)\

#define		TM_Sched_PortDefPrio5(operation, ...) \
    TM_REG_ROW_##operation( Deficit, 0, 19 , 0x1, __VA_ARGS__)\

#define		TM_Sched_PortDefPrio6(operation, ...) \
    TM_REG_ROW_##operation( Deficit, 0, 19 , 0x1, __VA_ARGS__)\

#define		TM_Sched_PortDefPrio7(operation, ...) \
    TM_REG_ROW_##operation( Deficit, 0, 19 , 0x1, __VA_ARGS__)\

#endif



