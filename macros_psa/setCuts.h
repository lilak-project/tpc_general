TCut cutt_tbRem_dTb_topPattern("cutt_tbRem_dTb_topPattern","(ampSim<4096)&&(tbSim-int(tbSim))>0.915&&(tbSim-int(tbSim))<0.921&&(tbSim-tbRec)>-0.0875&&(tbSim-tbRec)<-0.074");
TCut cutt_ampRem_dTb_sideBranch("cutt_ampRec_ampSim_split","(ampSim>5000)&&(tbSim-int(tbSim))<0.1&&(tbSim-tbRec)<0.45");
TCut cutt_ampRec_ampSim_split("cutt_ampRec_ampSim_split","(ampSim>5000)&&(ampRec<ampSim-500)");
TCut cutt_tbRem_dTb_leftCross("cutt_tbRem_dTb_leftCross","(ampSim>4096)&&(tbSim-int(tbSim))>0.15&&(tbSim-int(tbSim))<0.2&&(tbSim-tbRec)>-0.93&&(tbSim-tbRec)<-0.62");
