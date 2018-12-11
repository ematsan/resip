-- phpMyAdmin SQL Dump
-- version 4.6.6deb5
-- https://www.phpmyadmin.net/
--
-- Host: localhost:3306
-- Generation Time: Dec 11, 2018 at 02:07 PM
-- Server version: 5.7.24-0ubuntu0.18.04.1
-- PHP Version: 7.2.10-0ubuntu0.18.04.1

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `registrar`
--

-- --------------------------------------------------------

--
-- Table structure for table `tauthorization`
--

DROP TABLE IF EXISTS `tauthorization`;
CREATE TABLE `tauthorization` (
  `fidauth` int(11) NOT NULL,
  `fidudfk` int(11) DEFAULT NULL,
  `fpassword` varchar(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `tauthorization`
--

INSERT INTO `tauthorization` (`fidauth`, `fidudfk`, `fpassword`) VALUES
(1, 1, '312607ddd85fd7bfd1fd71e5423ce256'),
(4, 1, 'be534acd70c061ea0b274c19235eca10');

-- --------------------------------------------------------

--
-- Table structure for table `tdomain`
--

DROP TABLE IF EXISTS `tdomain`;
CREATE TABLE `tdomain` (
  `fiddomain` int(11) NOT NULL,
  `fdomain` varchar(255) NOT NULL,
  `fidrealm` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `tdomain`
--

INSERT INTO `tdomain` (`fiddomain`, `fdomain`, `fidrealm`) VALUES
(1, 'localhost', 5),
(2, '127.0.0.1', 5),
(4, 'test.ua', 5),
(5, 'test.server.ua', 5),
(6, '192.168.64.85', NULL),
(7, 'my.test.ua', NULL),
(8, 'sip.test.us', NULL);

-- --------------------------------------------------------

--
-- Table structure for table `tforward`
--

DROP TABLE IF EXISTS `tforward`;
CREATE TABLE `tforward` (
  `fidforward` int(11) NOT NULL,
  `fiddomainfk` int(11) NOT NULL,
  `fidprotocolfk` int(11) NOT NULL,
  `fport` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `tforward`
--

INSERT INTO `tforward` (`fidforward`, `fiddomainfk`, `fidprotocolfk`, `fport`) VALUES
(1, 2, 1, 0),
(2, 2, 1, 5080),
(3, 5, 1, 0),
(4, 6, 1, 60704),
(5, 6, 1, 46780);

-- --------------------------------------------------------

--
-- Table structure for table `tprotocol`
--

DROP TABLE IF EXISTS `tprotocol`;
CREATE TABLE `tprotocol` (
  `fidprotocol` int(11) NOT NULL,
  `fprotocol` varchar(20) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `tprotocol`
--

INSERT INTO `tprotocol` (`fidprotocol`, `fprotocol`) VALUES
(1, 'sip');

-- --------------------------------------------------------

--
-- Table structure for table `tregistrar`
--

DROP TABLE IF EXISTS `tregistrar`;
CREATE TABLE `tregistrar` (
  `fidreg` int(11) NOT NULL,
  `fidudfk` int(11) NOT NULL,
  `fcallid` varchar(255) NOT NULL,
  `fidmainfk` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `tregistrar`
--

INSERT INTO `tregistrar` (`fidreg`, `fidudfk`, `fcallid`, `fidmainfk`) VALUES
(6, 2, 'F6jcUB3I7viCsVWfm3kd6A..', 3),
(7, 4, 'QumqdZzL9Uc6K1VsG37epg..', 3),
(8, 3, 'SX8xuE1wthRBvMK4HB6ZIg..', 3),
(9, 6, '_95_P-t16KiwQGXCb8i07Q..', 3),
(10, 7, 'FPiyXzgphvaOPi-FxSbPZw..', 3),
(11, 5, '649237458', 5),
(12, 7, '1500290073', 7),
(13, 4, '1667116031', 4);

-- --------------------------------------------------------

--
-- Table structure for table `troute`
--

DROP TABLE IF EXISTS `troute`;
CREATE TABLE `troute` (
  `fidroute` int(11) NOT NULL,
  `fidregfk` int(11) NOT NULL,
  `fidforwardfk` int(11) NOT NULL,
  `ftime` datetime NOT NULL,
  `fexpires` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `troute`
--

INSERT INTO `troute` (`fidroute`, `fidregfk`, `fidforwardfk`, `ftime`, `fexpires`) VALUES
(7, 6, 2, '2018-12-11 12:05:56', 3600),
(8, 7, 2, '2018-12-11 11:10:07', 3600),
(9, 8, 2, '2018-12-11 11:10:10', 3600),
(10, 9, 2, '2018-12-11 11:20:16', 3600),
(11, 10, 2, '2018-12-11 12:00:37', 3600),
(12, 10, 3, '2018-12-11 11:24:04', 3600),
(13, 11, 5, '2018-12-11 11:54:13', 3600),
(14, 12, 3, '2018-12-11 11:25:40', 3600),
(15, 12, 5, '2018-12-11 11:54:42', 3600),
(16, 13, 5, '2018-12-11 11:56:09', 3600);

-- --------------------------------------------------------

--
-- Table structure for table `tuser`
--

DROP TABLE IF EXISTS `tuser`;
CREATE TABLE `tuser` (
  `fiduser` int(11) NOT NULL,
  `fname` varchar(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `tuser`
--

INSERT INTO `tuser` (`fiduser`, `fname`) VALUES
(1, 'yffulf'),
(4, 'admin'),
(5, 'test'),
(7, 'fluffy');

-- --------------------------------------------------------

--
-- Table structure for table `tuserdomain`
--

DROP TABLE IF EXISTS `tuserdomain`;
CREATE TABLE `tuserdomain` (
  `fidud` int(11) NOT NULL,
  `fiduserfk` int(11) NOT NULL,
  `fiddomainfk` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `tuserdomain`
--

INSERT INTO `tuserdomain` (`fidud`, `fiduserfk`, `fiddomainfk`) VALUES
(1, 5, 5),
(2, 5, 1),
(3, 5, 2),
(4, 5, 4),
(5, 5, 6),
(6, 5, 7),
(7, 5, 8);

--
-- Indexes for dumped tables
--

--
-- Indexes for table `tauthorization`
--
ALTER TABLE `tauthorization`
  ADD PRIMARY KEY (`fidauth`),
  ADD KEY `fidud` (`fidudfk`);

--
-- Indexes for table `tdomain`
--
ALTER TABLE `tdomain`
  ADD PRIMARY KEY (`fiddomain`),
  ADD UNIQUE KEY `fdomain` (`fdomain`),
  ADD KEY `fidrealm` (`fidrealm`);

--
-- Indexes for table `tforward`
--
ALTER TABLE `tforward`
  ADD PRIMARY KEY (`fidforward`),
  ADD KEY `fidprotocol` (`fidprotocolfk`),
  ADD KEY `fiddomain` (`fiddomainfk`);

--
-- Indexes for table `tprotocol`
--
ALTER TABLE `tprotocol`
  ADD PRIMARY KEY (`fidprotocol`);

--
-- Indexes for table `tregistrar`
--
ALTER TABLE `tregistrar`
  ADD PRIMARY KEY (`fidreg`),
  ADD KEY `fiduser` (`fidudfk`),
  ADD KEY `fidmain` (`fidmainfk`);

--
-- Indexes for table `troute`
--
ALTER TABLE `troute`
  ADD PRIMARY KEY (`fidroute`),
  ADD KEY `fidreg` (`fidregfk`),
  ADD KEY `fidforward` (`fidforwardfk`);

--
-- Indexes for table `tuser`
--
ALTER TABLE `tuser`
  ADD PRIMARY KEY (`fiduser`);

--
-- Indexes for table `tuserdomain`
--
ALTER TABLE `tuserdomain`
  ADD PRIMARY KEY (`fidud`),
  ADD KEY `fiddomainfk` (`fiddomainfk`),
  ADD KEY `fiduserfk` (`fiduserfk`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `tauthorization`
--
ALTER TABLE `tauthorization`
  MODIFY `fidauth` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=5;
--
-- AUTO_INCREMENT for table `tdomain`
--
ALTER TABLE `tdomain`
  MODIFY `fiddomain` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=9;
--
-- AUTO_INCREMENT for table `tforward`
--
ALTER TABLE `tforward`
  MODIFY `fidforward` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=6;
--
-- AUTO_INCREMENT for table `tprotocol`
--
ALTER TABLE `tprotocol`
  MODIFY `fidprotocol` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;
--
-- AUTO_INCREMENT for table `tregistrar`
--
ALTER TABLE `tregistrar`
  MODIFY `fidreg` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=14;
--
-- AUTO_INCREMENT for table `troute`
--
ALTER TABLE `troute`
  MODIFY `fidroute` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=17;
--
-- AUTO_INCREMENT for table `tuser`
--
ALTER TABLE `tuser`
  MODIFY `fiduser` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=10;
--
-- AUTO_INCREMENT for table `tuserdomain`
--
ALTER TABLE `tuserdomain`
  MODIFY `fidud` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=8;
--
-- Constraints for dumped tables
--

--
-- Constraints for table `tauthorization`
--
ALTER TABLE `tauthorization`
  ADD CONSTRAINT `tauthorization_ibfk_1` FOREIGN KEY (`fidudfk`) REFERENCES `tuserdomain` (`fidud`) ON DELETE SET NULL ON UPDATE CASCADE;

--
-- Constraints for table `tdomain`
--
ALTER TABLE `tdomain`
  ADD CONSTRAINT `tdomain_ibfk_1` FOREIGN KEY (`fidrealm`) REFERENCES `tdomain` (`fiddomain`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Constraints for table `tforward`
--
ALTER TABLE `tforward`
  ADD CONSTRAINT `tforward_ibfk_1` FOREIGN KEY (`fidprotocolfk`) REFERENCES `tprotocol` (`fidprotocol`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tforward_ibfk_2` FOREIGN KEY (`fiddomainfk`) REFERENCES `tdomain` (`fiddomain`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Constraints for table `tregistrar`
--
ALTER TABLE `tregistrar`
  ADD CONSTRAINT `tregistrar_ibfk_2` FOREIGN KEY (`fidudfk`) REFERENCES `tuserdomain` (`fidud`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tregistrar_ibfk_3` FOREIGN KEY (`fidmainfk`) REFERENCES `tuserdomain` (`fidud`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Constraints for table `troute`
--
ALTER TABLE `troute`
  ADD CONSTRAINT `troute_ibfk_1` FOREIGN KEY (`fidregfk`) REFERENCES `tregistrar` (`fidreg`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `troute_ibfk_2` FOREIGN KEY (`fidforwardfk`) REFERENCES `tforward` (`fidforward`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Constraints for table `tuserdomain`
--
ALTER TABLE `tuserdomain`
  ADD CONSTRAINT `tuserdomain_ibfk_1` FOREIGN KEY (`fiddomainfk`) REFERENCES `tdomain` (`fiddomain`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tuserdomain_ibfk_2` FOREIGN KEY (`fiduserfk`) REFERENCES `tuser` (`fiduser`) ON DELETE CASCADE ON UPDATE CASCADE;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
