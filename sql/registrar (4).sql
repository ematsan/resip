-- phpMyAdmin SQL Dump
-- version 4.6.6deb5
-- https://www.phpmyadmin.net/
--
-- Host: localhost:3306
-- Generation Time: Dec 04, 2018 at 03:03 PM
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

CREATE TABLE `tauthorization` (
  `fidauth` int(11) NOT NULL,
  `fiduser` int(11) NOT NULL,
  `fidrealm` int(11) NOT NULL,
  `fpassword` varchar(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `tauthorization`
--

INSERT INTO `tauthorization` (`fidauth`, `fiduser`, `fidrealm`, `fpassword`) VALUES
(1, 1, 1, '312607ddd85fd7bfd1fd71e5423ce256'),
(2, 1, 1, 'test'),
(3, 6, 1, 'be534acd70c061ea0b274c19235eca10');

-- --------------------------------------------------------

--
-- Table structure for table `tdomain`
--

CREATE TABLE `tdomain` (
  `fiddomain` int(11) NOT NULL,
  `fdomain` varchar(255) NOT NULL,
  `fidrealm` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `tdomain`
--

INSERT INTO `tdomain` (`fiddomain`, `fdomain`, `fidrealm`) VALUES
(1, 'localhost', 1),
(2, '127.0.0.1', 1),
(3, '127.0.0.57', 1),
(4, 'test.ua', 1);

-- --------------------------------------------------------

--
-- Table structure for table `tforward`
--

CREATE TABLE `tforward` (
  `fidforward` int(11) NOT NULL,
  `faddress` varchar(255) DEFAULT NULL,
  `fiddomain` int(11) NOT NULL,
  `fidprotocol` int(11) NOT NULL,
  `fip` varchar(50) DEFAULT NULL,
  `fport` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `tprotocol`
--

CREATE TABLE `tprotocol` (
  `fidprotocol` int(11) NOT NULL,
  `fprotocol` varchar(20) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `trealm`
--

CREATE TABLE `trealm` (
  `fidrealm` int(11) NOT NULL,
  `frealm` varchar(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `trealm`
--

INSERT INTO `trealm` (`fidrealm`, `frealm`) VALUES
(1, 'test.server.ua');

-- --------------------------------------------------------

--
-- Table structure for table `tregistrar`
--

CREATE TABLE `tregistrar` (
  `fidreg` int(11) NOT NULL,
  `fiduser` int(11) NOT NULL,
  `fiddomain` int(11) NOT NULL,
  `fcallid` varchar(255) NOT NULL,
  `fidmain` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `tregistrar`
--

INSERT INTO `tregistrar` (`fidreg`, `fiduser`, `fiddomain`, `fcallid`, `fidmain`) VALUES
(1, 2, 2, 'bfbEg-UXlxM28sN10dtf-A..', 2),
(2, 2, 2, '04AaPfgQ8b3ds3QBXA6LqQ..', 2);

-- --------------------------------------------------------

--
-- Table structure for table `troute`
--

CREATE TABLE `troute` (
  `fidroute` int(11) NOT NULL,
  `fidreg` int(11) NOT NULL,
  `fidforward` int(11) NOT NULL,
  `ftime` date NOT NULL,
  `fexpires` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `tuser`
--

CREATE TABLE `tuser` (
  `fiduser` int(11) NOT NULL,
  `fname` varchar(255) NOT NULL,
  `fiddomain` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `tuser`
--

INSERT INTO `tuser` (`fiduser`, `fname`, `fiddomain`) VALUES
(1, 'yffulf', 1),
(2, 'yffulf', 2),
(3, 'admin', 1),
(4, 'admin', 2),
(5, 'test', 1),
(6, 'test', 2),
(7, 'fluffy', 2);

--
-- Indexes for dumped tables
--

--
-- Indexes for table `tauthorization`
--
ALTER TABLE `tauthorization`
  ADD PRIMARY KEY (`fidauth`),
  ADD KEY `fiduser` (`fiduser`),
  ADD KEY `fiddomain` (`fidrealm`);

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
  ADD KEY `fidprotocol` (`fidprotocol`),
  ADD KEY `fiddomain` (`fiddomain`);

--
-- Indexes for table `tprotocol`
--
ALTER TABLE `tprotocol`
  ADD PRIMARY KEY (`fidprotocol`);

--
-- Indexes for table `trealm`
--
ALTER TABLE `trealm`
  ADD PRIMARY KEY (`fidrealm`),
  ADD UNIQUE KEY `frealm` (`frealm`);

--
-- Indexes for table `tregistrar`
--
ALTER TABLE `tregistrar`
  ADD PRIMARY KEY (`fidreg`),
  ADD KEY `fiduser` (`fiduser`),
  ADD KEY `fiddomain` (`fiddomain`),
  ADD KEY `fidmain` (`fidmain`);

--
-- Indexes for table `troute`
--
ALTER TABLE `troute`
  ADD PRIMARY KEY (`fidroute`),
  ADD KEY `fidreg` (`fidreg`),
  ADD KEY `fidforward` (`fidforward`);

--
-- Indexes for table `tuser`
--
ALTER TABLE `tuser`
  ADD PRIMARY KEY (`fiduser`),
  ADD KEY `fiddomain` (`fiddomain`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `tauthorization`
--
ALTER TABLE `tauthorization`
  MODIFY `fidauth` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=4;
--
-- AUTO_INCREMENT for table `tdomain`
--
ALTER TABLE `tdomain`
  MODIFY `fiddomain` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=5;
--
-- AUTO_INCREMENT for table `tforward`
--
ALTER TABLE `tforward`
  MODIFY `fidforward` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `tprotocol`
--
ALTER TABLE `tprotocol`
  MODIFY `fidprotocol` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `trealm`
--
ALTER TABLE `trealm`
  MODIFY `fidrealm` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;
--
-- AUTO_INCREMENT for table `tregistrar`
--
ALTER TABLE `tregistrar`
  MODIFY `fidreg` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=3;
--
-- AUTO_INCREMENT for table `troute`
--
ALTER TABLE `troute`
  MODIFY `fidroute` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `tuser`
--
ALTER TABLE `tuser`
  MODIFY `fiduser` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=8;
--
-- Constraints for dumped tables
--

--
-- Constraints for table `tauthorization`
--
ALTER TABLE `tauthorization`
  ADD CONSTRAINT `tauthorization_ibfk_1` FOREIGN KEY (`fiduser`) REFERENCES `tuser` (`fiduser`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tauthorization_ibfk_2` FOREIGN KEY (`fidrealm`) REFERENCES `trealm` (`fidrealm`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Constraints for table `tdomain`
--
ALTER TABLE `tdomain`
  ADD CONSTRAINT `tdomain_ibfk_1` FOREIGN KEY (`fidrealm`) REFERENCES `trealm` (`fidrealm`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Constraints for table `tforward`
--
ALTER TABLE `tforward`
  ADD CONSTRAINT `tforward_ibfk_1` FOREIGN KEY (`fidprotocol`) REFERENCES `tprotocol` (`fidprotocol`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tforward_ibfk_2` FOREIGN KEY (`fiddomain`) REFERENCES `tdomain` (`fiddomain`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Constraints for table `tregistrar`
--
ALTER TABLE `tregistrar`
  ADD CONSTRAINT `tregistrar_ibfk_1` FOREIGN KEY (`fiduser`) REFERENCES `tuser` (`fiduser`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tregistrar_ibfk_2` FOREIGN KEY (`fidmain`) REFERENCES `tuser` (`fiduser`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tregistrar_ibfk_3` FOREIGN KEY (`fiddomain`) REFERENCES `tdomain` (`fiddomain`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Constraints for table `troute`
--
ALTER TABLE `troute`
  ADD CONSTRAINT `troute_ibfk_1` FOREIGN KEY (`fidreg`) REFERENCES `tregistrar` (`fidreg`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `troute_ibfk_2` FOREIGN KEY (`fidforward`) REFERENCES `tforward` (`fidforward`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Constraints for table `tuser`
--
ALTER TABLE `tuser`
  ADD CONSTRAINT `tuser_ibfk_1` FOREIGN KEY (`fiddomain`) REFERENCES `tdomain` (`fiddomain`) ON DELETE CASCADE ON UPDATE CASCADE;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
