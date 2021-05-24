/*
 Navicat Premium Data Transfer

 Source Server         : testserver
 Source Server Type    : MySQL
 Source Server Version : 50562
 Source Host           : 5.135.217.85:3306
 Source Schema         : anticheat

 Target Server Type    : MySQL
 Target Server Version : 50562
 File Encoding         : 65001

 Date: 06/03/2019 02:57:18
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for cheat_blacklist_1
-- ----------------------------
DROP TABLE IF EXISTS `cheat_blacklist_1`;
CREATE TABLE `cheat_blacklist_1`  (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `md5_hash` varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `details` varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = MyISAM AUTO_INCREMENT = 4 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of cheat_blacklist_1
-- ----------------------------
INSERT INTO `cheat_blacklist_1` VALUES (1, 'f173f26b26470c41adbaa7f59627f3dc', 'm2bob.exe_20190217');
INSERT INTO `cheat_blacklist_1` VALUES (2, 'a09a863f35ca071d025d9086005b0f09', 'lalaker1.app_20190217');
INSERT INTO `cheat_blacklist_1` VALUES (3, '7362840264f515c736a9fd7d164db961', 'm24pro_3.160');

SET FOREIGN_KEY_CHECKS = 1;
