pipeline {
    agent any 

    stages {
        stage('SCANOSS Analysis') {
            agent {
                docker {
                    image 'python:3.9-slim'
                    args '-u root --platform linux/arm64'
                }
            }
            steps {
                script {
                    echo "--- Instalando SCANOSS (Limpio) ---"
                    sh "pip install scanoss"

                    withCredentials([string(credentialsId: 'scanoss-token', variable: 'TOKEN')]) {
                        echo "--- Escaneando y Generando CycloneDX ---"
                        
                        sh "python3 -m scanoss.cli scan . --apiurl https://api.scanoss.com --key ${TOKEN} --output results.json"
                        
                        
                        sh "python3 -m scanoss.cli convert --input results.json --format cyclonedx --output results.xml"
                    }

                    echo "--- Mostrando Resultados en Jenkins ---"
                    
                    recordIssues(
                        tools: [cycloneDx(pattern: 'results.xml', id: 'scanoss', name: 'SCANOSS Analysis')],
                        qualityGates: [[threshold: 1, type: 'TOTAL', criticality: 'UNSTABLE']]
                    )
                    
                    archiveArtifacts artifacts: "results.json, results.xml", allowEmptyArchive: true
                }
            }
        }
    }
}