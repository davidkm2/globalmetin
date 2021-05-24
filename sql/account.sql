/*
 Navicat MySQL Data Transfer

 Source Server         : local
 Source Server Type    : MySQL
 Source Server Version : 100316
 Source Host           : localhost:3306
 Source Schema         : account

 Target Server Type    : MySQL
 Target Server Version : 100316
 File Encoding         : 65001

 Date: 29/07/2019 13:07:52
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for account
-- ----------------------------
DROP TABLE IF EXISTS `account`;
CREATE TABLE `account`  (
  `id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `login` varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '' COMMENT 'LOGIN_MAX_LEN=30',
  `password` varchar(42) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '' COMMENT 'PASSWD_MAX_LEN=16; default 45 size',
  `social_id` varchar(7) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '',
  `email` varchar(100) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT '',
  `status` varchar(8) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT 'OK',
  `availDt` datetime(0) NOT NULL,
  `create_time` datetime(0) NOT NULL,
  `last_play` datetime(0) NOT NULL,
  `gold_expire` datetime(0) NOT NULL DEFAULT '2029-03-24 15:46:06',
  `silver_expire` datetime(0) NOT NULL DEFAULT '2029-03-24 15:46:06',
  `safebox_expire` datetime(0) NOT NULL DEFAULT '2029-03-24 15:46:06',
  `autoloot_expire` datetime(0) NOT NULL DEFAULT '2029-03-24 15:46:06',
  `fish_mind_expire` datetime(0) NOT NULL DEFAULT '2029-03-24 15:46:06',
  `marriage_fast_expire` datetime(0) NOT NULL DEFAULT '2029-03-24 15:46:06',
  `money_drop_rate_expire` datetime(0) NOT NULL DEFAULT '2029-03-24 15:46:06',
  `real_name` varchar(16) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT '',
  `question1` varchar(56) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `answer1` varchar(56) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `question2` varchar(56) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `answer2` varchar(56) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `cash` int(11) NULL DEFAULT 0,
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `login`(`login`) USING BTREE,
  INDEX `social_id`(`social_id`) USING BTREE
) ENGINE = MyISAM AUTO_INCREMENT = 8 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of account
-- ----------------------------
INSERT INTO `account` VALUES (1, 'admin', '*00A51F3F48415C7D4E8908980D443C29C69B60C9', '1234567', '', 'OK', '0000-00-00 00:00:00', '0000-00-00 00:00:00', '2019-06-23 01:37:44', '2029-03-24 15:46:06', '2029-03-24 15:46:06', '2029-03-24 15:46:06', '2029-03-24 15:46:06', '2029-03-24 15:46:06', '2029-03-24 15:46:06', '2029-03-24 15:46:06', '', NULL, NULL, NULL, NULL, 200);
INSERT INTO `account` VALUES (3, 'tests', '*00A51F3F48415C7D4E8908980D443C29C69B60C9', '1234567', '', 'OK', '0000-00-00 00:00:00', '0000-00-00 00:00:00', '2019-06-22 15:37:46', '2029-03-24 15:46:06', '2029-03-24 15:46:06', '2029-03-24 15:46:06', '2029-03-24 15:46:06', '2029-03-24 15:46:06', '2029-03-24 15:46:06', '2029-03-24 15:46:06', '', NULL, NULL, NULL, NULL, 0);
INSERT INTO `account` VALUES (7, 'koray', '*33C8AA3592E11BCA039FB09DB191473139A9AAAC', '', '', 'OK', '0000-00-00 00:00:00', '0000-00-00 00:00:00', '0000-00-00 00:00:00', '2029-03-24 15:46:06', '2029-03-24 15:46:06', '2029-03-24 15:46:06', '2029-03-24 15:46:06', '2029-03-24 15:46:06', '2029-03-24 15:46:06', '2029-03-24 15:46:06', '', NULL, NULL, NULL, NULL, 0);

SET FOREIGN_KEY_CHECKS = 1;
