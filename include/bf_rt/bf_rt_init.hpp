/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */


/** @file bf_rt_init.hpp
 *
 *  @brief Contains BF-RT Dev Manager APIs. These APIs help manage BfRtInfo
 *  \n objects with respect to the devices and programs in the ecosystem
 */
#ifndef _BF_RT_INIT_HPP_
#define _BF_RT_INIT_HPP_

#ifdef __cplusplus
extern "C" {
#endif

#include <dvm/bf_drv_intf.h>

#ifdef __cplusplus
}
#endif

#include <functional>
#include <memory>
#include <mutex>

// bf_rt includes
#include <bf_rt/bf_rt_info.hpp>

namespace bfrt {

// Forward declarations
class BfRtDevMgrImpl;

/**
 * @brief Class to manage BFRTinfo objects per (dev_id, program_name).<br>
 * <B>Creation: </B> Singleton....
 */
class BfRtDevMgr {
 public:
  /**
   * @brief Get the singleton ojbect
   *
   * @return Ref to Singleton BfRtDevMgr object
   */
  static BfRtDevMgr &getInstance();

  /**
   * @brief Get the BfRtInfo object corresponding to the (device_id,
   * program name)
   *
   * @param[in] dev_id Device ID
   * @param[in] prog_name Name of the P4 program
   * @param[out] bfrt_info BfRtInfo Obj associated with the Device
   *    and the program name
   *
   * @return Status of the API call
   */
  bf_status_t bfRtInfoGet(const bf_dev_id_t &dev_id,
                          const std::string &prog_name,
                          const BfRtInfo **bfrt_info) const;

  /**
   * @brief Get a list of all device IDs currently added
   *
   * @param[out] device_id_list Set Contains list of device
   * IDs that are present.
   *
   * @return Status of the API call
   */
  bf_status_t bfRtDeviceIdListGet(std::set<bf_dev_id_t> *device_id_list) const;

  /**
   * @brief Get a vector of loaded p4 program names on a particular device
   *
   * @param[in] dev_id Device ID
   * @param[out] p4_names Vector containing const string references to the
   * P4 names loaded on the device
   *
   * @return Status of the API call
   */
  bf_status_t bfRtInfoP4NamesGet(
      const bf_dev_id_t &dev_id,
      std::vector<std::reference_wrapper<const std::string>> &p4_names);

  /**
   * @brief Get a vector of fixed json path names for a device
   *
   * @param[in] dev_id Device ID
   * @param[out] fixed_file_vec Vector containing const string references to the
   * fixed json path names
   *
   * @return Status of the API call
   */
  bf_status_t fixedFilePathsGet(
      const bf_dev_id_t &dev_id,
      std::vector<std::reference_wrapper<const std::string>> &fixed_file_vec);

  BfRtDevMgr(BfRtDevMgr const &) = delete;
  BfRtDevMgr(BfRtDevMgr &&) = delete;
  BfRtDevMgr &operator=(const BfRtDevMgr &) = delete;
  BfRtDevMgr &operator=(BfRtDevMgr &&) = delete;

 private:
  BfRtDevMgr();
  static BfRtDevMgr *dev_mgr_instance;
  static std::mutex dev_mgr_instance_mutex;

  std::unique_ptr<BfRtDevMgrImpl> dev_mgr_impl_;
  const BfRtDevMgrImpl *devMgrImpl() const { return dev_mgr_impl_.get(); }
  BfRtDevMgrImpl *devMgrImpl() { return dev_mgr_impl_.get(); }

  friend bf_status_t bf_rt_virtual_device_add(
      bf_dev_id_t dev_id,
      bf_dev_type_t dev_type,
      bf_device_profile_t *dev_profile,
      bf_dev_init_mode_t warm_init_mode);
  friend bf_status_t bf_rt_device_add(bf_dev_id_t dev_id,
                                      bf_dev_family_t dev_family,
                                      bf_device_profile_t *dev_profile,
                                      bf_dma_info_t *dma_info,
                                      bf_dev_init_mode_t warm_init_mode);
  friend bf_status_t bf_rt_device_remove(bf_dev_id_t dev_id);
};  // BfRtDevMgr

/**
 * @brief Class to manage initialization of BF-RT <br>
 * <B>Creation: </B> Cannot be created
 */
class BfRtInit {
 public:
  /**
   * @brief Bf Rt Module Init API. This function needs to be called to
   * initialize BF-RT. Some specific managers can be specified to be skipped
   * BFRT initialization. This allows BFRT session layer to not know about these
   * managers.
   * Recommendation is not to skip any unless user knows exactly
   * what they are doing.
   *
   * @param[in] pkt_mgr_skip Skip Packet mgr
   * @param[in] mc_mgr_skip Skip Multicast mgr
   * @param[in] port_mgr_skip Skip Port mgr
   * @param[in] traffic_mgr_skip Skip Traffic mgr
   * @return Status of the API call
   */
  static bf_status_t bfRtModuleInit(bool pkt_mgr_skip,
                                    bool mc_mgr_skip,
                                    bool port_mgr_skip,
                                    bool traffic_mgr_skip);
};  // BfRtInit

}  // namespace bfrt

#endif  // _BF_RT_INIT_HPP_
