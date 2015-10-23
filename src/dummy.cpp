/*
 * Copyright 2014 Instituto de Sistemas e Robotica, Instituto Superior Tecnico
 *
 * This file is part of RoAH RSBB Comm ROS.
 *
 * RoAH RSBB Comm ROS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RoAH RSBB Comm ROS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with RoAH RSBB Comm ROS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <memory>

#include <boost/noncopyable.hpp>

#include <ros/ros.h>

#include <std_msgs/UInt32.h>
#include <roah_rsbb_comm_ros/Benchmark.h>
#include <roah_rsbb_comm_ros/BenchmarkState.h>

#include <std_srvs/Empty.h>
#include <roah_rsbb_comm_ros/ResultHOPF.h>



using namespace std;
using namespace ros;



class Benchmark
  : boost::noncopyable
{
  protected:
    NodeHandle nh_;

    void
    end_prepare()
    {
      if (ros::service::waitForService ("/roah_rsbb/end_prepare", 100)) {
        std_srvs::Empty s;
        if (! ros::service::call ("/roah_rsbb/end_prepare", s)) {
          ROS_ERROR ("Error calling service /roah_rsbb/end_prepare");
        }
      }
      else {
        ROS_ERROR ("Could not find service /roah_rsbb/end_prepare");
      }
    }

    void
    end_execute()
    {
      if (ros::service::waitForService ("/roah_rsbb/end_execute", 100)) {
        std_srvs::Empty s;
        if (! ros::service::call ("/roah_rsbb/end_execute", s)) {
          ROS_ERROR ("Error calling service /roah_rsbb/end_execute");
        }
      }
      else {
        ROS_ERROR ("Could not find service /roah_rsbb/end_execute");
      }
    }

    virtual void
    prepare()
    {
      Duration (3, 0).sleep();

      end_prepare();
    }

    virtual void
    execute()
    {
      Duration (3, 0).sleep();

      end_execute();
    }

  private:
    Subscriber benchmark_state_sub_;

    Publisher messages_saved_pub_;

    void
    benchmark_state_callback (roah_rsbb_comm_ros::BenchmarkState::ConstPtr const& msg)
    {
      switch (msg->benchmark_state) {
        case roah_rsbb_comm_ros::BenchmarkState::STOP:
          break;
        case roah_rsbb_comm_ros::BenchmarkState::PREPARE:
          prepare();
          break;
        case roah_rsbb_comm_ros::BenchmarkState::EXECUTE:
          execute();
          break;
      }
    }

  public:
    Benchmark()
      : nh_()
      , benchmark_state_sub_ (nh_.subscribe ("/roah_rsbb/benchmark/state", 1, &Benchmark::benchmark_state_callback, this))
      , messages_saved_pub_ (nh_.advertise<std_msgs::UInt32> ("/roah_rsbb/messages_saved", 1, true))
    {
      // This should reflect the real number or size of messages saved.
      std_msgs::UInt32 messages_saved_msg;
      messages_saved_msg.data = 1;
      messages_saved_pub_.publish (messages_saved_msg);
    }
};



class HGTKMH
  : public Benchmark
{
  public:
    HGTKMH()
    {
    }
};



class HWV
  : public Benchmark
{
  public:
    HWV()
    {
    }
};



class HCFGAC
  : public Benchmark
{
  public:
    HCFGAC()
    {
    }
};



class HOPF
  : public Benchmark
{
  public:
    HOPF()
    {
    }

    void
    execute()
    {
      Duration (3, 0).sleep();

      if (ros::service::waitForService ("/roah_rsbb/end_execute", 100)) {
        roah_rsbb_comm_ros::ResultHOPF s;
        s.request.object_class = "cups";
        s.request.object_name = "red_cup";
        s.request.object_pose.x = 0.1;
        s.request.object_pose.y = 0.2;
        s.request.object_pose.theta = 1.23;

        if (! ros::service::call ("/roah_rsbb/end_execute", s)) {
          ROS_ERROR ("Error calling service /roah_rsbb/end_execute");
        }
      }
      else {
        ROS_ERROR ("Could not find service /roah_rsbb/end_execute");
      }
    }
};



class HNF
  : public Benchmark
{
  public:
    HNF()
    {
    }
};



class HSUF
  : public Benchmark
{
  public:
    HSUF()
    {
    }
};



class DummyRobot
{
    NodeHandle nh_;

    Subscriber benchmark_sub_;
    roah_rsbb_comm_ros::Benchmark::_benchmark_type last_benchmark_;

    unique_ptr<Benchmark> benchmark_;

    void
    benchmark_callback (roah_rsbb_comm_ros::Benchmark::ConstPtr const& msg)
    {
      if (last_benchmark_ == msg->benchmark) {
        return;
      }

      last_benchmark_ = msg->benchmark;

      // Destroy the old before creating a new. Just for precaution.
      benchmark_.reset();

      switch (msg->benchmark) {
        case roah_rsbb_comm_ros::Benchmark::NONE:
          break;
        case roah_rsbb_comm_ros::Benchmark::HGTKMH:
          benchmark_.reset (new HGTKMH());
          break;
        case roah_rsbb_comm_ros::Benchmark::HWV:
          benchmark_.reset (new HWV());
          break;
        case roah_rsbb_comm_ros::Benchmark::HCFGAC:
          benchmark_.reset (new HCFGAC());
          break;
        case roah_rsbb_comm_ros::Benchmark::HOPF:
          benchmark_.reset (new HOPF());
          break;
        case roah_rsbb_comm_ros::Benchmark::HNF:
          benchmark_.reset (new HNF());
          break;
        case roah_rsbb_comm_ros::Benchmark::HSUF:
          benchmark_.reset (new HSUF());
          break;
      }
    }

  public:
    DummyRobot()
      : nh_()
      , benchmark_sub_ (nh_.subscribe ("/roah_rsbb/benchmark", 1, &DummyRobot::benchmark_callback, this))
      , last_benchmark_ (roah_rsbb_comm_ros::Benchmark::NONE)
      , benchmark_()
    {
    }
};



int main (int argc, char** argv)
{
  init (argc, argv, "dummy_roah_robot");

  DummyRobot node;

  spin();
  return 0;
}
