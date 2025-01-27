
#ifndef ttm_hpp
#define ttm_hpp

#include <QDialog>

#include "ui_ttm.h"

#include "../../lib/multiIndi.hpp"

namespace xqt 
{
   
class ttm : public QDialog, public multiIndiSubscriber
{
   Q_OBJECT
   
protected:
   
   std::string m_procName;
   
   std::string m_appState;
   
   int m_naxes {2};
   
   double m_pos_1 {0.0};;
   double m_scale_1 {0.01};
   
   double m_pos_2 {0.0};;
   double m_scale_2 {0.01};
   
   double m_pos_3 {0.0};;
   double m_scale_3 {0.01};
   
   std::string m_shmimName;
   bool m_flatSet;
   bool m_testSet;
   
public:
   ttm( std::string & procName,
             QWidget * Parent = 0, 
             Qt::WindowFlags f = 0
           );
   
   ~ttm();
   
   int subscribe( multiIndiPublisher * publisher );
                                   
   int handleDefProperty( const pcf::IndiProperty & ipRecv /**< [in] the property which has changed*/);
   
   int handleSetProperty( const pcf::IndiProperty & ipRecv /**< [in] the property which has changed*/);
   
   void sendNewPos1(double np);
   void sendNewPos2(double np);
   void sendNewPos3(double np);
   
public slots:
   void updateGUI();
   
   void on_button_home_pressed();
   void on_button_zero_pressed();
   void on_button_flat_pressed();
   void on_button_release_pressed();

   void on_button_scale_1_pressed();
   void on_button_up_1_pressed();
   void on_button_down_1_pressed();
   
   void on_button_scale_2_pressed();
   void on_button_up_2_pressed();
   void on_button_down_2_pressed();
   
   void on_button_scale_3_pressed();
   void on_button_up_3_pressed();
   void on_button_down_3_pressed();
   
   
private:
     
   Ui::ttm ui;
};
   
ttm::ttm( std::string & procName,
                    QWidget * Parent, 
                    Qt::WindowFlags f) : QDialog(Parent, f), m_procName{procName}
{
   ui.setupUi(this);
   
   setWindowTitle(QString(m_procName.c_str()));
   ui.button_scale_1->setText(QString::number(m_scale_1));
   ui.label_device_status->setProperty("isStatus", true);
   ui.lcd_1->setProperty("isStatus", true);
   ui.lcd_2->setProperty("isStatus", true);
   ui.lcd_3->setProperty("isStatus", true);
   
   ui.label_device->setText(m_procName.c_str());
   ui.label_device_status->setText("unkown");
   
}
   
ttm::~ttm()
{
}

int ttm::subscribe( multiIndiPublisher * publisher )
{
   if(!publisher) return -1;
   
   publisher->subscribeProperty(this, m_procName, "fsm");
   publisher->subscribeProperty(this, m_procName, "pos_1");
   publisher->subscribeProperty(this, m_procName, "pos_2");
   publisher->subscribeProperty(this, m_procName, "pos_3");
   
   publisher->subscribeProperty(this, m_procName, "sm_shmimName");
   publisher->subscribeProperty(this, m_procName, "flat");
   publisher->subscribeProperty(this, m_procName, "flat_shmim");
   publisher->subscribeProperty(this, m_procName, "flat_set");
   publisher->subscribeProperty(this, m_procName, "test");
   publisher->subscribeProperty(this, m_procName, "test_shmim");
   publisher->subscribeProperty(this, m_procName, "test_set");
   return 0;
}
   
int ttm::handleDefProperty( const pcf::IndiProperty & ipRecv)
{  
   return handleSetProperty(ipRecv);
   
   return 0;
}

int ttm::handleSetProperty( const pcf::IndiProperty & ipRecv)
{  
   if(ipRecv.getDevice() != m_procName) return 0;
   
   if(ipRecv.getName() == "fsm")
   {
      if(ipRecv.find("state"))
      {
         m_appState = ipRecv["state"].get<std::string>();
      }
   }
   else if(ipRecv.getName() == "pos_1")
   {
      if(ipRecv.find("current"))
      {
         m_pos_1 = ipRecv["current"].get<double>();
      }
   }
   else if(ipRecv.getName() == "pos_2")
   {
      if(ipRecv.find("current"))
      {
         m_pos_2 = ipRecv["current"].get<double>();
      }
   }
   else if(ipRecv.getName() == "pos_3")
   {
      m_naxes = 3;
      if(ipRecv.find("current"))
      {
         m_pos_3 = ipRecv["current"].get<double>();
      }
   }
   else if(ipRecv.getName() == "flat_set")
   {
      if(ipRecv.find("toggle"))
      {
         if(ipRecv["toggle"] == pcf::IndiElement::On) m_flatSet = true;
         else m_flatSet = false;
      }
   }
   else if(ipRecv.getName() == "test_set")
   {
      if(ipRecv.find("toggle"))
      {
         if(ipRecv["toggle"] == pcf::IndiElement::On) m_testSet = true;
         else m_testSet = false;
      }
   }
   else if(ipRecv.getName() == "sm_shmimName")
   {
      if(ipRecv.find("name"))
      {
         m_shmimName = ipRecv["name"].get<std::string>();
      }
   }

   updateGUI();
   
   return 0;
   
}

void ttm::sendNewPos1(double np)
{
   try
   {
      pcf::IndiProperty ipFreq(pcf::IndiProperty::Number);

      ipFreq.setDevice(m_procName);
      ipFreq.setName("pos_1");
      ipFreq.add(pcf::IndiElement("current"));
      ipFreq.add(pcf::IndiElement("target"));
      ipFreq["current"] = np;
      ipFreq["target"] = np;
   
      sendNewProperty(ipFreq);   
   }
   catch(...)
   {
      std::cerr << "libcommon INDI exception.  going on. (" << __FILE__ << " " << __LINE__ << "\n";
   }
}

void ttm::sendNewPos2(double np)
{
   try
   {
      pcf::IndiProperty ipFreq(pcf::IndiProperty::Number);

      ipFreq.setDevice(m_procName);
      ipFreq.setName("pos_2");
      ipFreq.add(pcf::IndiElement("current"));
      ipFreq.add(pcf::IndiElement("target"));
      ipFreq["current"] = np;
      ipFreq["target"] = np;
   
      sendNewProperty(ipFreq);   
   }
   catch(...)
   {
      std::cerr << "libcommon INDI exception.  going on. (" << __FILE__ << " " << __LINE__ << "\n";
   }
}

void ttm::sendNewPos3(double np)
{
   if(m_naxes < 3) return;
   
   try
   {
      pcf::IndiProperty ipFreq(pcf::IndiProperty::Number);

      ipFreq.setDevice(m_procName);
      ipFreq.setName("pos_3");
      ipFreq.add(pcf::IndiElement("current"));
      ipFreq.add(pcf::IndiElement("target"));
      ipFreq["current"] = np;
      ipFreq["target"] = np;
   
      sendNewProperty(ipFreq);   
   }
   catch(...)
   {
      std::cerr << "libcommon INDI exception.  going on. (" << __FILE__ << " " << __LINE__ << "\n";
   }
}

void ttm::updateGUI()
{
   
   
   if(m_naxes == 3)
   {
      ui.label_1->setText("Piston");
      ui.label_2->setText("Tip");
      ui.label_3->setText("Tilt");
   }
   
   ui.lcd_1->display(m_pos_1);
   ui.lcd_2->display(m_pos_2);
   ui.lcd_3->display(m_pos_3);
   
   if(m_appState == "NOTHOMED") 
   {
      ui.label_device_status->setText("RIP");
      
      ui.label_1->setEnabled(false);
      ui.lcd_1->setEnabled(false);
      ui.lineEdit_1->setEnabled(false);
      ui.button_up_1->setEnabled(false);
      ui.button_scale_1->setEnabled(false);
      ui.button_down_1->setEnabled(false);
         
      ui.label_2->setEnabled(false);
      ui.lcd_2->setEnabled(false);
      ui.lineEdit_2->setEnabled(false);
      ui.button_up_2->setEnabled(false);
      ui.button_scale_2->setEnabled(false);
      ui.button_down_2->setEnabled(false);
      
      ui.label_3->setEnabled(false);
      ui.lcd_3->setEnabled(false);
      ui.lineEdit_3->setEnabled(false);
      ui.button_up_3->setEnabled(false);
      ui.button_scale_3->setEnabled(false);
      ui.button_down_3->setEnabled(false);
      
      ui.button_home->setEnabled(true);
      ui.button_zero->setEnabled(false);
      ui.button_flat->setEnabled(false);
      ui.button_release->setEnabled(false);  
   }
   else if(m_appState == "HOMING") 
   {
      ui.label_device_status->setText("SETTING");
      
      ui.label_1->setEnabled(false);
      ui.lcd_1->setEnabled(true);
      ui.lineEdit_1->setEnabled(false);
      ui.button_up_1->setEnabled(false);
      ui.button_scale_1->setEnabled(false);
      ui.button_down_1->setEnabled(false);
         
      ui.label_2->setEnabled(false);
      ui.lcd_2->setEnabled(true);
      ui.lineEdit_2->setEnabled(false);
      ui.button_up_2->setEnabled(false);
      ui.button_scale_2->setEnabled(false);
      ui.button_down_2->setEnabled(false);
      
      ui.label_3->setEnabled(false);
      if(m_naxes == 2) ui.lcd_3->setEnabled(false);
      else ui.lcd_3->setEnabled(true);
      ui.lineEdit_3->setEnabled(false);
      ui.button_up_3->setEnabled(false);
      ui.button_scale_3->setEnabled(false);
      ui.button_down_3->setEnabled(false);
      
      ui.button_home->setEnabled(false);
      ui.button_zero->setEnabled(false);
      ui.button_flat->setEnabled(false);
      ui.button_release->setEnabled(false);  
   }
   else if(m_appState == "READY" || m_appState == "OPERATING")
   {
      ui.label_device_status->setText("SET");
      
      ui.label_1->setEnabled(true);
      ui.lcd_1->setEnabled(true);
      ui.lineEdit_1->setEnabled(true);
      ui.button_up_1->setEnabled(true);
      ui.button_scale_1->setEnabled(true);
      ui.button_down_1->setEnabled(true);
         
      ui.label_2->setEnabled(true);
      ui.lcd_2->setEnabled(true);
      ui.lineEdit_2->setEnabled(true);
      ui.button_up_2->setEnabled(true);
      ui.button_scale_2->setEnabled(true);
      ui.button_down_2->setEnabled(true);
      
      if(m_naxes == 2)
      {
         ui.label_3->setEnabled(false);
         ui.lcd_3->setEnabled(false);
         ui.lineEdit_3->setEnabled(false);
         ui.button_up_3->setEnabled(false);
         ui.button_scale_3->setEnabled(false);
         ui.button_down_3->setEnabled(false);
      }
      else
      {
         ui.label_3->setEnabled(true);
         ui.lcd_3->setEnabled(true);
         ui.lineEdit_3->setEnabled(true);
         ui.button_up_3->setEnabled(true);
         ui.button_scale_3->setEnabled(true);
         ui.button_down_3->setEnabled(true);
      }
      
      ui.button_home->setEnabled(false);
      ui.button_zero->setEnabled(true);
      ui.button_flat->setEnabled(true);
      ui.button_release->setEnabled(true);
   }
   else
   {
      ui.label_device_status->setText(m_appState.c_str());
      
      //Disable & zero all
      ui.label_1->setEnabled(false);
      ui.lcd_1->setEnabled(false);
      ui.lineEdit_1->setEnabled(false);
      ui.button_up_1->setEnabled(false);
      ui.button_scale_1->setEnabled(false);
      ui.button_down_1->setEnabled(false);
         
      ui.label_2->setEnabled(false);
      ui.lcd_2->setEnabled(false);
      ui.lineEdit_2->setEnabled(false);
      ui.button_up_2->setEnabled(false);
      ui.button_scale_2->setEnabled(false);
      ui.button_down_2->setEnabled(false);
      
      ui.label_3->setEnabled(false);
      ui.lcd_3->setEnabled(false);
      ui.lineEdit_3->setEnabled(false);
      ui.button_up_3->setEnabled(false);
      ui.button_scale_3->setEnabled(false);
      ui.button_down_3->setEnabled(false);
      
      ui.button_home->setEnabled(false);
      ui.button_zero->setEnabled(false);
      ui.button_flat->setEnabled(false);
      ui.button_release->setEnabled(false);

      return;
   }

   
} //updateGUI()

void ttm::on_button_home_pressed()
{
   pcf::IndiProperty ipFreq(pcf::IndiProperty::Switch);
   
   ipFreq.setDevice(m_procName);
   ipFreq.setName("initDM");
   ipFreq.add(pcf::IndiElement("request"));
   ipFreq["request"].setSwitchState(pcf::IndiElement::On);
    
   sendNewProperty(ipFreq);   
}

void ttm::on_button_zero_pressed()
{
   sendNewPos1(0.0);
   sendNewPos2(0.0);
   sendNewPos3(0.0);
}

void ttm::on_button_flat_pressed()
{
}

void ttm::on_button_release_pressed()
{
   pcf::IndiProperty ipFreq(pcf::IndiProperty::Switch);
   
   ipFreq.setDevice(m_procName);
   ipFreq.setName("releaseDM");
   ipFreq.add(pcf::IndiElement("request"));
   ipFreq["request"].setSwitchState(pcf::IndiElement::On);
    
   sendNewProperty(ipFreq);   
}

void ttm::on_button_scale_1_pressed()
{
   if(((int) (100*m_scale_1)) == 100)
   {
      m_scale_1 = 0.5;
   }
   else if(((int) (100*m_scale_1)) == 50)
   {
      m_scale_1 = 0.1;
   }
   else if(((int) (100*m_scale_1)) == 10)
   {
      m_scale_1 = 0.05;
   }
   else if(((int) (100*m_scale_1)) == 5)
   {
      m_scale_1 = 0.01;
   }
   else if(((int) (100*m_scale_1)) == 1)
   {
      m_scale_1 = 1.0;
   }
   
   char ss[5];
   snprintf(ss, 5, "%0.2f", m_scale_1);
   ui.button_scale_1->setText(ss);
}

void ttm::on_button_up_1_pressed()
{
   pcf::IndiProperty ip(pcf::IndiProperty::Number);
   
   ip.setDevice(m_procName);
   ip.setName("pos_1");
   ip.add(pcf::IndiElement("target"));
   ip["target"] = m_pos_1 + m_scale_1;
   
   sendNewProperty(ip);
}

void ttm::on_button_down_1_pressed()
{
   pcf::IndiProperty ip(pcf::IndiProperty::Number);
   
   ip.setDevice(m_procName);
   ip.setName("pos_1");
   ip.add(pcf::IndiElement("target"));
   ip["target"] = m_pos_1 - m_scale_1;
   
   sendNewProperty(ip);
}

void ttm::on_button_scale_2_pressed()
{
   if(((int) (100*m_scale_2)) == 100)
   {
      m_scale_2 = 0.5;
   }
   else if(((int) (100*m_scale_2)) == 50)
   {
      m_scale_2 = 0.1;
   }
   else if(((int) (100*m_scale_2)) == 10)
   {
      m_scale_2 = 0.05;
   }
   else if(((int) (100*m_scale_2)) == 5)
   {
      m_scale_2 = 0.01;
   }
   else if(((int) (100*m_scale_2)) == 1)
   {
      m_scale_2 = 1.0;
   }
   
   char ss[5];
   snprintf(ss, 5, "%0.2f", m_scale_2);
   ui.button_scale_2->setText(ss);
}

void ttm::on_button_up_2_pressed()
{
   pcf::IndiProperty ip(pcf::IndiProperty::Number);
   
   ip.setDevice(m_procName);
   ip.setName("pos_2");
   ip.add(pcf::IndiElement("target"));
   ip["target"] = m_pos_2 + m_scale_2;
   
   sendNewProperty(ip);
}

void ttm::on_button_down_2_pressed()
{
   pcf::IndiProperty ip(pcf::IndiProperty::Number);
   
   ip.setDevice(m_procName);
   ip.setName("pos_2");
   ip.add(pcf::IndiElement("target"));
   ip["target"] = m_pos_2 - m_scale_2;
   
   sendNewProperty(ip);
}

void ttm::on_button_scale_3_pressed()
{
   if(((int) (100*m_scale_3)) == 100)
   {
      m_scale_3 = 0.5;
   }
   else if(((int) (100*m_scale_3)) == 50)
   {
      m_scale_3 = 0.1;
   }
   else if(((int) (100*m_scale_3)) == 10)
   {
      m_scale_3 = 0.05;
   }
   else if(((int) (100*m_scale_3)) == 5)
   {
      m_scale_3 = 0.01;
   }
   else if(((int) (100*m_scale_3)) == 1)
   {
      m_scale_3 = 1.0;
   }
   
   char ss[5];
   snprintf(ss, 5, "%0.2f", m_scale_3);
   ui.button_scale_3->setText(ss);
}

void ttm::on_button_up_3_pressed()
{
   pcf::IndiProperty ip(pcf::IndiProperty::Number);
   
   ip.setDevice(m_procName);
   ip.setName("pos_3");
   ip.add(pcf::IndiElement("target"));
   ip["target"] = m_pos_3 + m_scale_3;
   
   sendNewProperty(ip);
}

void ttm::on_button_down_3_pressed()
{
   pcf::IndiProperty ip(pcf::IndiProperty::Number);
   
   ip.setDevice(m_procName);
   ip.setName("pos_3");
   ip.add(pcf::IndiElement("target"));
   ip["target"] = m_pos_3 - m_scale_3;
   
   sendNewProperty(ip);
}

// void ttm::on_button_gainScale_pressed()
// {
//    //Can only be 0.01, 0.05, or 0.1, but we make sure floating point doesn't scew us up.  
//    //the progresion is:
//    // 0.01->0.1->0.05->0.01
//    if(m_gainScale < 0.05) //0.01
//    {
//       m_gainScale = 0.1;
//    }
//    else if(m_gainScale < 0.1) //0.05
//    {
//       m_gainScale = 0.01;
//    }
//    else //0.1
//    {
//       m_gainScale = 0.05;
//    }
//    
//    ui.button_gainScale->setText(QString::number(m_gainScale));
// }
// 
// void ttm::on_button_gainUp_pressed()
// {
//    double ng = m_gain + m_gainScale;
//    sendNewGain(ng);
// }
// 
// void ttm::on_button_gainDown_pressed()
// {
//    double ng = m_gain - m_gainScale;
//    if(ng < 0.00001) ng = 0; //Stop floating point nonsense
//    sendNewGain(ng);
// }
// 
// void ttm::on_lineEdit_gain_returnPressed()
// {
//    double ng = ui.lineEdit_gain->text().toDouble();
//    
//    if(ng < 0) ng = 0;
// 
//    sendNewGain(ng);
//    
//    ui.lineEdit_gain->setText("");
// }
// 
// void ttm::on_button_zeroGain_pressed()
// {
//    sendNewGain(0.0);
// }
// 
// void ttm::on_button_multcoeffScale_pressed()
// {
//    //Can only be 0.001, 0.002, 0.005, or 0.01 but we make sure floating point doesn't scew us up.  
//    //the progresion is:
//    // 0.001->0.01->0.005->0.002->0.001
//    if(m_multcoeffScale < 0.002) //0.001
//    {
//       m_multcoeffScale = 0.01;
//    }
//    else if(m_multcoeffScale < 0.005) //0.002
//    {
//       m_multcoeffScale = 0.001;
//    }
//    else if(m_multcoeffScale < 0.01) //0.005
//    {
//       m_multcoeffScale = 0.002;
//    }
//    else //0.01
//    {
//       m_multcoeffScale = 0.005;
//    }
//    
//    ui.button_multcoeffScale->setText(QString::number(m_multcoeffScale));
// }
// 
// void ttm::on_button_multcoeffUp_pressed()
// {
//    double nm = m_multcoeff + m_multcoeffScale;
//    if(nm > 1) nm = 1;
//    sendNewMultCoeff(nm);
// }
// 
// void ttm::on_button_multcoeffDown_pressed()
// {
//    double nm = m_multcoeff - m_multcoeffScale;
//    if(nm < 0.00001) nm = 0; //Stop floating point nonsense
//    sendNewMultCoeff(nm);
// }
// 
// void ttm::on_lineEdit_multcoeff_returnPressed()
// {
//    double nm = ui.lineEdit_multcoeff->text().toDouble();
//    
//    if(nm < 0) nm = 0;
//    if(nm > 1) nm = 1;
//    
//    sendNewMultCoeff(nm);
//    
//    ui.lineEdit_multcoeff->setText("");
// }
// 
// void ttm::on_button_oneMultCoeff_pressed()
// {
//    sendNewMultCoeff(1.0);
// }
// 
// void ttm::on_slider_loop_sliderReleased()
// {
//    double relpos = ((double)(ui.slider_loop->sliderPosition() - ui.slider_loop->minimum()))/(ui.slider_loop->maximum() - ui.slider_loop->minimum());
//    
//    if(relpos > 0.1 && relpos < 0.9)
//    {
//       if(m_loopState)
//       {
//          ui.slider_loop->setSliderPosition(ui.slider_loop->maximum());
//       }
//       else
//       {
//          ui.slider_loop->setSliderPosition(ui.slider_loop->minimum());
//       }
//       return;
//    }
//    
//    ui.label_loop_state->setText("-----");
//    ui.label_loop_state->setEnabled(false);
//    ui.slider_loop->setEnabled(false);
//    m_loopWaiting = true;
//    
//    pcf::IndiProperty ipFreq(pcf::IndiProperty::Switch);
//    
//    ipFreq.setDevice(m_procName);
//    ipFreq.setName("loop_state");
//    ipFreq.add(pcf::IndiElement("toggle"));
//    
//    if(relpos >= 0.9)
//    {
//       ipFreq["toggle"] = pcf::IndiElement::On;
//    }
//    else
//    {
//       ipFreq["toggle"] = pcf::IndiElement::Off;
//    }
//    
//    sendNewProperty(ipFreq);
// }
// 
// void ttm::on_button_LoopZero_pressed()
// {
//    std::cerr << "loop zero\n";
// }

} //namespace xqt
   
#include "moc_ttm.cpp"

#endif
