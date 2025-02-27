/*!
 * \brief Unit tests for \ref CertificateGenerator
 *
 * \copyright Copyright (c) 2017-2022 Governikus GmbH & Co. KG, Germany
 */

#include "KeyPair.h"

#include "TlsChecker.h"

#include <QSslCertificateExtension>
#include <QtTest>

using namespace governikus;


class test_KeyPair
	: public QObject
{
	Q_OBJECT

	private Q_SLOTS:
		void validKey()
		{
			const KeyPair pair = KeyPair::generate();
			QVERIFY(pair.isValid());
			const auto& key = pair.getKey();
			QVERIFY(!key.isNull());
			QCOMPARE(key.length(), 2048);
			QCOMPARE(key.algorithm(), QSsl::Rsa);
			QCOMPARE(key.type(), QSsl::PrivateKey);
		}


		void multiKey()
		{
			const KeyPair pair1 = KeyPair::generate();
			QVERIFY(!pair1.getKey().isNull());

			const KeyPair pair2 = KeyPair::generate();
			QVERIFY(!pair2.getKey().isNull());

			QVERIFY(pair1.getKey() != pair2.getKey());
		}


		void validCertificate()
		{
			const KeyPair pair = KeyPair::generate();
			QVERIFY(pair.isValid());
			const auto& cert = pair.getCertificate();
			QVERIFY(!cert.isNull());
			QCOMPARE(cert.issuerInfo(QSslCertificate::CommonName).size(), 1);
			QCOMPARE(cert.issuerInfo(QSslCertificate::CommonName).at(0), QCoreApplication::applicationName());
			QCOMPARE(cert.issuerInfo(QSslCertificate::SerialNumber).size(), 1);
			QVERIFY(cert.issuerInfo(QSslCertificate::SerialNumber).at(0).size() > 0);
			QVERIFY(cert.extensions().isEmpty());
			QCOMPARE(cert.version(), QByteArray("1"));
			QVERIFY(cert.isSelfSigned());

			X509* rawCert = static_cast<X509*>(cert.handle());
			const qlonglong serialNumberValue = ASN1_INTEGER_get(X509_get_serialNumber(rawCert));
			QVERIFY(serialNumberValue > 0);
			QVERIFY(QByteArray::number(serialNumberValue).size() < 21);

			const auto& key = cert.publicKey();
			QCOMPARE(key.length(), 2048);
			QCOMPARE(key.algorithm(), QSsl::Rsa);
			QCOMPARE(key.type(), QSsl::PublicKey);
			QVERIFY(TlsChecker::hasValidCertificateKeyLength(cert));

			QVERIFY(cert.expiryDate().isValid());
			QVERIFY(cert.effectiveDate().isValid());
			QVERIFY(cert.expiryDate() > QDateTime::currentDateTime());
			QVERIFY(cert.effectiveDate() <= QDateTime::currentDateTime());
		}


		void multiCertificate()
		{
			const KeyPair pair1 = KeyPair::generate();
			QVERIFY(!pair1.getCertificate().isNull());

			const KeyPair pair2 = KeyPair::generate();
			QVERIFY(!pair2.getCertificate().isNull());

			QVERIFY(pair1.getCertificate() != pair2.getCertificate());
		}


};

QTEST_GUILESS_MAIN(test_KeyPair)
#include "test_KeyPair.moc"
